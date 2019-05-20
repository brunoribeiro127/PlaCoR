#pragma once

namespace cz
{
namespace rpc
{

class BaseOutProcessor
{
public:
	virtual ~BaseOutProcessor() {}
protected:

	template<typename R> friend class Call;
	template<typename L, typename R> friend struct Connection;

	template<typename F, typename H>
	void commit(Transport& transport, std::string group, uint32_t rpcid, Stream& data, H&& handler)
	{
		std::unique_lock<std::mutex> lk(m_mtx);
		Header hdr;
		hdr.bits.size = data.writeSize();
		hdr.bits.counter = ++m_replyIdCounter;
		hdr.bits.rpcid = rpcid;
		*reinterpret_cast<Header*>(data.ptr(0)) = hdr;
		m_replies[hdr.key()] = [handler = std::move(handler)](Stream* in, Header hdr)
		{
			using R = typename ParamTraits<typename FunctionTraits<F>::return_type>::store_type;
			if (in)
			{
				if (hdr.bits.success)
				{
					handler(Result<R>::fromStream((*in)));
				}
				else
				{
					std::string str;
					(*in) >> str;
					handler(Result<R>::fromException(std::move(str)));
				}
			}
			else
			{
				// if the stream is nullptr, it means the result is being aborted
				handler(Result<R>());
			}
		};
		lk.unlock();

		transport.send(group, data.extract());
	}

	void processReply(Stream& in, Header hdr)
	{
		std::function<void(Stream*, Header)> h;
		{
			std::unique_lock<std::mutex> lk(m_mtx);
			auto it = m_replies.find(hdr.key());
			assert(it != m_replies.end());
			h = std::move(it->second);
			m_replies.erase(it);
		}

		h(&in, hdr);
	}


	void abortReplies()
	{
		decltype(m_replies) replies;
		{
			std::unique_lock<std::mutex> lk(m_mtx);
			replies = std::move(m_replies);
		}

		for (auto&& r : replies)
		{
			r.second(nullptr, Header());
		}
	};

	std::mutex m_mtx;
	uint32_t m_replyIdCounter = 0;
	std::unordered_map<uint32_t, std::function<void(Stream*, Header)>> m_replies;
};

template<typename F>
class Call
{
private:
	using RType = typename FunctionTraits<F>::return_type;
	using RTraits = ParamTraits<RType>;
public:

	Call(Call&& other)
		: m_outer(other.m_outer)
		, m_transport(other.m_transport)
		, m_group(other.m_group)
		, m_rpcid(other.m_rpcid)
		, m_data(std::move(other.m_data))
	{
	}

	Call(const Call&) = delete;
	Call& operator=(const Call&) = delete;
	Call& operator=(Call&&) = delete;

	~Call()
	{
		if (m_data.writeSize() && !m_commited)
			async([](Result<typename RTraits::store_type>) {});
	}

	template<typename H>
	void async(H&& handler)
	{
		m_outer.commit<F>(m_transport, m_group, m_rpcid, m_data, std::forward<H>(handler));
		m_commited = true;
	}

	std::future<class Result<typename RTraits::store_type>> ft()
	{
		auto pr = std::make_shared<std::promise<Result<typename RTraits::store_type>>>();
		auto ft = pr->get_future();
		async([pr=std::move(pr)](Result<typename RTraits::store_type>&& res) 
		{
			pr->set_value(std::move(res));
		});

		return ft;
	}

protected:

	template<typename T> friend class OutProcessor;

	explicit Call(BaseOutProcessor& outer, Transport& transport, std::string const& group, uint32_t rpcid)
		: m_outer(outer), m_transport(transport), m_group(group), m_rpcid(rpcid)
	{
		m_data << Header(); // Reserve space for the header
	}

	template<typename... Args>
	void serializeParams(Args&&... args)
	{
		serializeMethod<F>(m_data, std::forward<Args>(args)...);
	}

	BaseOutProcessor& m_outer;
	Transport& m_transport;
	std::string m_group;
	uint32_t m_rpcid;
	Stream m_data;
	// Used in the destructor to do a commit with an empty handler if the rpc was not committed.
	bool m_commited = false;
};

namespace details
{
	// Signature of the generic RPC call. This helps reuse some of the code,
	// since it's just another function
	typedef Any(*GenericRPCFunc)(const std::string&, const std::vector<Any>&);
}

template<typename T>
class OutProcessor : public BaseOutProcessor
{
public:
	using Type = T;

	template<typename F, typename... Args>
	auto call(Transport& transport, std::string const& group, uint32_t rpcid, Args&&... args)
	{
		using Traits = FunctionTraits<F>;
		static_assert(
			std::is_member_function_pointer<F>::value &&
			std::is_base_of<typename Traits::class_type, Type>::value,
			"Not a member function of the wrapped class");
		Call<F> c(*this, transport, group, rpcid);
		c.serializeParams(std::forward<Args>(args)...);
		return std::move(c);
	}

	auto callGeneric(Transport& transport, std::string const& group, const std::string& name, const std::vector<Any>& args)
	{
		Call<details::GenericRPCFunc> c(*this, transport, group, (int)Table<T>::RPCId::genericRPC);
		c.serializeParams(name, args);
		return std::move(c);
	}

protected:

};

// Specialization for when there is no outgoing RPC calls
// If we have no outgoing RPC calls, receiving a reply is therefore an error.
template <>
class OutProcessor<void>
{
  public:
	OutProcessor() {}
	void processReply(Stream&, Header) { assert(0 && "Incoming replies not allowed for OutProcessor<void>"); }
	void abortReplies() {}
};


class BaseInProcessor
{
public:
	BaseInProcessor(void* obj)
		: m_data(obj)
	{
		m_data.authPassed = m_data.objData.getAuthToken() == "" ? true : false;
	}

	virtual ~BaseInProcessor()
	{}

	void setAuthToken(std::string tk)
	{
		m_data.objData.setAuthToken(std::move(tk));
	}

protected:
	InProcessorData m_data;
};

template<typename T>
class InProcessor : public BaseInProcessor
{
public:
	using Type = T;
	InProcessor(Type* obj)
		: BaseInProcessor(obj)
		, m_obj(*obj)
	{
	}

	void processCall(Transport& transport, std::string group, Stream& in, Header hdr)
	{
		auto&& info = Table<Type>::get(hdr.bits.rpcid);
		info->dispatcher(m_obj, in, m_data, transport, group, hdr);
	}

protected:
	Type& m_obj;
};

template<>
class InProcessor<void>
{
public:
	InProcessor(void*) { }
	void processCall(Transport& trp, std::string group, Stream& in, Header hdr)
	{
		//assert(0 && "Incoming RPC not allowed for void local type");
		details::Send::error(trp, group, hdr, "Peer doesn't have an object to process RPC calls");
	}
};

#define CZRPC_FUNCTION(...) __VA_ARGS__

#define CZRPC_CALL(con, group, rpcid, func, ...)                          				 \
    (con).call<decltype(&std::decay<decltype(con)>::type::Remote::func)>( 				 \
        *(con).transport,                                                 				 \
    	group, 														      				 \
        (uint32_t)cz::rpc::Table<std::decay<decltype(con)>::type::Remote>::RPCId::rpcid, \
        ##__VA_ARGS__)

#define CZRPC_CALLGENERIC(con, group, name, ...) \
	(con).callGeneric(*(con).transport, group, name, ##__VA_ARGS__)

} // namespace rpc
} // namespace cz



#ifndef RPCTABLE_CLASS
	#error "Macro RPCTABLE_CLASS needs to be defined"
#endif
#ifndef RPCTABLE_CONTENTS
	#error "Macro RPCTABLE_CONTENTS needs to be defined"
#endif


#define RPCTABLE_TOOMANYRPCS_STRINGIFY(arg) #arg
#define RPCTABLE_TOOMANYRPCS(arg) RPCTABLE_TOOMANYRPCS_STRINGIFY(arg)

template<> class cz::rpc::Table<RPCTABLE_CLASS> : cz::rpc::TableImpl<RPCTABLE_CLASS>
{
public:
	using Type = RPCTABLE_CLASS;
    #undef REGISTERRPC
	#define REGISTERRPC(rsc, rpcid, ...) rpcid,
	enum class RPCId {
		genericRPC,
		RPCTABLE_CONTENTS
		NUMRPCS
	};

	Table()
	{
		registerGenericRPC();
		static_assert((unsigned)((int)RPCId::NUMRPCS-1)<(1<<Header::kRPCIdBits),
			RPCTABLE_TOOMANYRPCS(Too many RPCs registered for class RPCTABLE_CLASS));
		#undef REGISTERRPC
		#define REGISTERRPC(rsc, rpcid, ...) registerRPC((uint32_t)RPCId::rpcid, #__VA_ARGS__, &Type::__VA_ARGS__);
		RPCTABLE_CONTENTS
	}

	static const Info* get(uint32_t rpcid)
	{
		static Table<RPCTABLE_CLASS> tbl;
		assert(tbl.isValid(rpcid));
		return static_cast<Info*>(tbl.m_rpcs[rpcid].get());
	}
};


//#undef REGISTERRPC
#undef RPCTABLE_START
#undef RPCTABLE_END
//#undef RPCTABLE_CLASS
//#undef RPCTABLE_CONTENTS
#undef RPCTABLE_TOOMANYRPCS_STRINGIFY
#undef RPCTABLE_TOOMANYRPCS

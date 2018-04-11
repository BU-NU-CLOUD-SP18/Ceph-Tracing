# Compare Blkin and Jaeger



## Blkin
This is the Endpoint Class from the Ztracer namespace
```C++
class Endpoint : private blkin_endpoint {
    private:
	string _ip; // storage for blkin_endpoint.ip, see copy_ip()
	string _name; // storage for blkin_endpoint.name, see copy_name()

	friend class Trace;
    public:
	Endpoint(const char *name)
	    {
		blkin_init_endpoint(this, "0.0.0.0", 0, name);
	    }
	Endpoint(const char *ip, int port, const char *name)
	    {
		blkin_init_endpoint(this, ip, port, name);
	    }

	// copy constructor and operator need to account for ip/name storage
	Endpoint(const Endpoint &rhs) : _ip(rhs._ip), _name(rhs._name)
	    {
		blkin_init_endpoint(this, _ip.size() ? _ip.c_str() : rhs.ip,
				    rhs.port,
				    _name.size() ? _name.c_str(): rhs.name);
	    }
	const Endpoint& operator=(const Endpoint &rhs)
	    {
		_ip.assign(rhs._ip);
		_name.assign(rhs._name);
		blkin_init_endpoint(this, _ip.size() ? _ip.c_str() : rhs.ip,
				    rhs.port,
				    _name.size() ? _name.c_str() : rhs.name);
		return *this;
	    }

	// Endpoint assumes that ip and name will be string literals, and
	// avoids making a copy and freeing on destruction.  if you need to
	// initialize Endpoint with a temporary string, copy_ip/copy_name()
	// will store it in a std::string and assign from that
	void copy_ip(const string &newip)
	    {
		_ip.assign(newip);
		ip = _ip.c_str();
	    }
	void copy_name(const string &newname)
	    {
		_name.assign(newname);
		name = _name.c_str();
	    }

	void copy_address_from(const Endpoint *endpoint)
	    {
		_ip.assign(endpoint->ip);
		ip = _ip.c_str();
		port = endpoint->port;
	    }
	void share_address_from(const Endpoint *endpoint)
	    {
		ip = endpoint->ip;
		port = endpoint->port;
	    }
	void set_port(int p) { port = p; }
    };

```
### link
blkin/blkin-lib/ztracer.hpp   

## Jaeger
```C++
class Endpoint {
 public:

  static const char* ascii_fingerprint; // = "66357C747F937372BB32913F3D70EB58";
  static const uint8_t binary_fingerprint[16]; // = {0x66,0x35,0x7C,0x74,0x7F,0x93,0x73,0x72,0xBB,0x32,0x91,0x3F,0x3D,0x70,0xEB,0x58};

  Endpoint(const Endpoint&);
  Endpoint& operator=(const Endpoint&);
  Endpoint() : ipv4(0), port(0), service_name(), ipv6() {
  }

  virtual ~Endpoint() throw();
  int32_t ipv4;
  int16_t port;
  std::string service_name;
  std::string ipv6;

  _Endpoint__isset __isset;

  void __set_ipv4(const int32_t val);

  void __set_port(const int16_t val);

  void __set_service_name(const std::string& val);

  void __set_ipv6(const std::string& val);

  bool operator == (const Endpoint & rhs) const
  {
    if (!(ipv4 == rhs.ipv4))
      return false;
    if (!(port == rhs.port))
      return false;
    if (!(service_name == rhs.service_name))
      return false;
    if (__isset.ipv6 != rhs.__isset.ipv6)
      return false;
    else if (__isset.ipv6 && !(ipv6 == rhs.ipv6))
      return false;
    return true;
  }
  bool operator != (const Endpoint &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Endpoint & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const Endpoint& obj);
};

```
### link
jaeger-client-cpp/src/jaegertracing/thrift-gen/zipkincore_types.h


## Blkin
This is the init() function from the Trace class in Ztracer namespace
```C++
// (re)initialize a Trace with an optional parent
	int init(const char *name, const Endpoint *ep,
		 const Trace *parent = NULL)
	    {
		if (parent && parent->valid())
		    return blkin_init_child(this, parent,
					    ep ? : parent->endpoint, name);

		return blkin_init_new_trace(this, name, ep);
	    }

	// (re)initialize a Trace from blkin_trace_info
	int init(const char *name, const Endpoint *ep,
		 const blkin_trace_info *i, bool child=false)
	    {
		if (child)
		    return blkin_init_child_info(this, i, ep, _name.c_str());

		return blkin_set_trace_properties(this, i, _name.c_str(), ep);
	    }
```
### link
blkin/blkin-lib/ztracer.hpp        

## Jaeger
```C++
Tracer::AnalyzedReferences
Tracer::analyzeReferences(const std::vector<OpenTracingRef>& references) const
{
    AnalyzedReferences result;
    auto hasParent = false;
    const auto* parent = result._parent;
    for (auto&& ref : references) {
        const auto* ctx = dynamic_cast<const SpanContext*>(ref.second);

        if (!ctx) {
            _logger->error("Reference contains invalid type of SpanReference");
            continue;
        }

        if (!ctx->isValid() && !ctx->isDebugIDContainerOnly() &&
            ctx->baggage().empty()) {
            continue;
        }

        result._references.emplace_back(Reference(*ctx, ref.first));

        if (!hasParent) {
            parent = ctx;
            hasParent =
                (ref.first == opentracing::SpanReferenceType::ChildOfRef);
        }
    }

    if (!hasParent && parent && parent->isValid()) {
        // Use `FollowsFromRef` in place of `ChildOfRef`.
        hasParent = true;
    }

    if (hasParent) {
        assert(parent);
        result._parent = parent;
    }

    return result;
}

```
### link
jaeger-client-cpp/src/jaegertracing/Tracer.h

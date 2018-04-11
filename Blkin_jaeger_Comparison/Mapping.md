
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

```
### link
jaeger-client-cpp/src/jaegertracing/Tracer.h

## Blkin
This is the Trace() class present in the Ztracer namespace

```C++
// record timestamp annotations
	void event(const char *event) const
	    {
		if (valid())
		    BLKIN_TIMESTAMP(this, endpoint, event);
	    }
	void event(const char *event, const Endpoint *ep) const
	    {
		if (valid())
		    BLKIN_TIMESTAMP(this, ep, event);
	    }
```
### link
blkin/blkin-lib/ztracer.hpp   

## Jaeger
```C++

```
### link







/*
 * Copyright 2014 Marios Kogias <marioskogias@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 *   1. Redistributions of source code must retain the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer.
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef ZTRACER_H

#define ZTRACER_H

#include <string>

extern "C" {
#include <zipkin_c.h>
}

#include <jaegertracing/Tracer.h> /* mania */
#include <jaegertracing/Logging.h>/* bowen */
#include <jaegertracing/Tag.h> /* bowen */
#include <jaegertracing/Span.h>

namespace ZTracer {
using std::string;

const char* const CLIENT_SEND = "cs";
const char* const CLIENT_RECV = "cr";
const char* const SERVER_SEND = "ss";
const char* const SERVER_RECV = "sr";
const char* const WIRE_SEND = "ws";
const char* const WIRE_RECV = "wr";
const char* const CLIENT_SEND_FRAGMENT = "csf";
const char* const CLIENT_RECV_FRAGMENT = "crf";
const char* const SERVER_SEND_FRAGMENT = "ssf";
const char* const SERVER_RECV_FRAGMENT = "srf";

static inline int ztrace_init() {
	return blkin_init();
}

class Endpoint : private blkin_endpoint {
private:
	string _ip; // storage for blkin_endpoint.ip, see copy_ip()
	string _name; // storage for blkin_endpoint.name, see copy_name()

	friend class Trace;
public:
	Endpoint(const char *name) {
		blkin_init_endpoint(this, "0.0.0.0", 0, name);
	}
	Endpoint(const char *ip, int port, const char *name) {
		blkin_init_endpoint(this, ip, port, name);
	}

	// copy constructor and operator need to account for ip/name storage
	Endpoint(const Endpoint &rhs) : _ip(rhs._ip), _name(rhs._name) {
		blkin_init_endpoint(this, _ip.size() ? _ip.c_str() : rhs.ip,
		                    rhs.port,
		                    _name.size() ? _name.c_str(): rhs.name);
	}
	const Endpoint& operator=(const Endpoint &rhs) {
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
	void copy_ip(const string &newip) {
		_ip.assign(newip);
		ip = _ip.c_str();
	}
	void copy_name(const string &newname) {
		_name.assign(newname);
		name = _name.c_str();
	}

	void copy_address_from(const Endpoint *endpoint) {
		_ip.assign(endpoint->ip);
		ip = _ip.c_str();
		port = endpoint->port;
	}
	void share_address_from(const Endpoint *endpoint) {
		ip = endpoint->ip;
		port = endpoint->port;
	}
	void set_port(int p) {
		port = p;
	}
};


/*mania*/
class Trace: public jaegertracing::Tracer {
private:
public:
	Trace() {
		//In ztracer Used as zero-initialize so valid() returns false
		//we match it to
		//Jaeger throw invalid argument: 'no service name provided'
		/*Bowen thoughts*/
		//name = NULL;
		this->endpoint = NULL;
		//Tracer::make(name,endpoint);
		/* call tracer::init(....)*/
	}

	Trace(const char *name, const Endpoint *ep, const Trace *parent = NULL) {
		Tracer::make(name,ep2config(ep));// Bowen
	}

	Trace(const Trace &rhs) : _name(rhs._name) {
		//Sorry, what is this?
	}

	/*Bowen*/
	// Converting endpoint (Blkin) to Config (Jaeger)
	Tracer::Config ep2config(const Endpoint *endpoint) {
		//
		//Put into Config
		//It is parsed by Jaeger from the following function
		//Thus we are making it into a string
		//const auto hostPort =
		//utils::yaml::findOrDefault<std::string>(configYAML, "hostPort", "");
		//char[] IPnPort = endpoint->ip+":"+endpoint->port;
		
			constexpr auto kConfigYAML = R"cfg(
		disabled: true
		sampler:
		    type: probabilistic
		    param: 0.001
		reporter:
		    queueSize: 100
		    bufferFlushInterval: 10
		    logSpans: false
		    localAgentHostPort: 127.0.0.1:6831
		headers:
		    jaegerDebugHeader: debug-id
		    jaegerBaggageHeader: baggage
		    TraceContextHeaderName: trace-id
		    traceBaggageHeaderPrefix: "testctx-"
		baggage_restrictions:
		    denyBaggageOnInitializationFailure: false
		    hostPort: endpoint->ip+":"+endpoint->port
		    refreshInterval: 60
		)cfg";
			const auto config = jaegertracing::Config::parse(YAML::Load(kConfigYAML));

		
		return config
	}

	// (re)initialize a Trace with an optional parent --------------------------------------------------- Bowen
	int init(const char *name, const Endpoint *ep,
	         const Trace *parent = NULL) { // IDK why is the original return type int
		//opentracing::StartSpanOptions options;
		// option seems to only care about parent trace
		// With this, i will start with defult
		// I dont know where to put endpoint
		Tracer::std::unique_ptr<Tracer::Span> span(static_cast<Tracer::Span*>
		                                   (this->StartSpanWithOptions(name, {}).release())); // should be parent, but not risking it
		return 0;// for success
	}

	// (re)initialize a Trace from blkin_trace_info
	int init(const char *name, const Endpoint *ep,
	         const blkin_trace_info *i, bool child=false) {
		opentracing::StartSpanOptions options;
		options.start_steady_timestamp = Tracer::SteadyClock::now();
		const SpanContext parentCtx(
		    i->trace_id, i->span_id, i->parent_span_id, 1234, SpanContext::StrMap());

		if (child) {
			options.references.emplace_back(opentracing::SpanReferenceType::ChildOfRef,
			                                &parentCtx);
			Tracer::std::unique_ptr<Span> span(static_cast<Span*>
			                                   (this->StartSpanWithOptions(name, options).release()));
			return 0;
		}
		options.references.emplace_back(
		    opentracing::SpanReferenceType::FollowsFromRef, &parentCtx);
		Tracer::std::unique_ptr<Span> span(static_cast<Span*>
		                                   (this->StartSpanWithOptions(name, options).release()));
		return 0;
	}
	// end init ------------------------------------------------------------ Bowen
	// start of valid -----------------------------Bowen
	// return true if the Trace has been initialized
	bool valid() const {
		return endpoint != NULL;
	}
	operator bool() const {
		return valid();
	}
	// end of valid --------------------------------Bowen
	//Start of Keyval -----------------------------Bowen
	//Keyval -- ithink it is LogRecord in jaeger C++
	void keyval(const char *key, const char *val) const {
		if (valid()) // assume this is span
			this.Span->Log({ { key, val } });//dont know if the syntax is correct ------------------------------------------------------ possible error
		//BLKIN_KEYVAL_STRING(this, endpoint, key, val);
	}
	void keyval(const char *key, int64_t val) const {
		if (valid())// assume this is span
			this.Span->Log({ { key, val } });
		//BLKIN_KEYVAL_INTEGER(this, endpoint, key, val);
	}
	void keyval(const char *key, const char *val, const Endpoint *ep) const {
		if (valid())
			this.Span->Log({ { key, val } });
		//BLKIN_KEYVAL_STRING(this, ep, key, val);
	}
	void keyval(const char *key, int64_t val, const Endpoint *ep) const {
		if (valid())
			this.Span->Log({ { key, val } });
		//BLKIN_KEYVAL_INTEGER(this, ep, key, val);
	}
	/*	keyval() ---> log_kv
	 *	event ---> tag
	 *	every where u see init you should call
	*/

	void event(const char *event) const {
		if (valid())//think this is span
			this.Span->SetOperationName(
			    event); // we are not caring about endpoint, since ,,, i dont have a place to put it and it should be in span
		//BLKIN_TIMESTAMP(this, endpoint, event);
	}
	void event(const char *event, const Endpoint *ep) const {
		if (valid())
			this.Span->SetOperationName(event);
		//BLKIN_TIMESTAMP(this, ep, event);
	}

};
// End of Bowen think for Event function ----------- Bowen
class TraceB : private blkin_trace {
private:
	string _name; // storage for blkin_trace.name, see copy_name()

public:
	// default constructor zero-initializes blkin_trace valid()
	// will return false on a default-constructed Trace until
	// init()
	Trace() {
		// zero-initialize so valid() returns false
		name = NULL;
		info.trace_id = 0;
		info.span_id = 0;
		info.parent_span_id = 0;
		endpoint = NULL;
	}

	// construct a Trace with an optional parent
	Trace(const char *name, const Endpoint *ep, const Trace *parent = NULL) {
		if (parent && parent->valid()) {
			blkin_init_child(this, parent, ep ? : parent->endpoint,
			                 name);
		} else {
			blkin_init_new_trace(this, name, ep);
		}
	}

	// construct a Trace from blkin_trace_info
	Trace(const char *name, const Endpoint *ep,
	      const blkin_trace_info *i, bool child=false) {
		if (child)
			blkin_init_child_info(this, i, ep, name);
		else {
			blkin_init_new_trace(this, name, ep);
			set_info(i);
		}
	}

	// copy constructor and operator need to account for name storage
	Trace(const Trace &rhs) : _name(rhs._name) {
		name = _name.size() ? _name.c_str() : rhs.name;
		info = rhs.info;
		endpoint = rhs.endpoint;
	}
	const Trace& operator=(const Trace &rhs) {
		_name.assign(rhs._name);
		name = _name.size() ? _name.c_str() : rhs.name;
		info = rhs.info;
		endpoint = rhs.endpoint;
		return *this;
	}

	// return true if the Trace has been initialized
	bool valid() const {
		return info.trace_id != 0;
	}
	operator bool() const {
		return valid();
	}

	// (re)initialize a Trace with an optional parent
	int init(const char *name, const Endpoint *ep,
	         const Trace *parent = NULL) {
		if (parent && parent->valid())
			return blkin_init_child(this, parent,
			                        ep ? : parent->endpoint, name);

		return blkin_init_new_trace(this, name, ep);
	}

	// (re)initialize a Trace from blkin_trace_info
	int init(const char *name, const Endpoint *ep,
	         const blkin_trace_info *i, bool child=false) {
		if (child)
			return blkin_init_child_info(this, i, ep, _name.c_str());

		return blkin_set_trace_properties(this, i, _name.c_str(), ep);
	}

	// Trace assumes that name will be a string literal, and avoids
	// making a copy and freeing on destruction.  if you need to
	// initialize Trace with a temporary string, copy_name() will store
	// it in a std::string and assign from that
	void copy_name(const string &newname) {
		_name.assign(newname);
		name = _name.c_str();
	}

	const blkin_trace_info* get_info() const {
		return &info;
	}
	void set_info(const blkin_trace_info *i) {
		info = *i;
	}

	// record key-value annotations
	void keyval(const char *key, const char *val) const {
		if (valid())
			BLKIN_KEYVAL_STRING(this, endpoint, key, val);
	}
	void keyval(const char *key, int64_t val) const {
		if (valid())
			BLKIN_KEYVAL_INTEGER(this, endpoint, key, val);
	}
	void keyval(const char *key, const char *val, const Endpoint *ep) const {
		if (valid())
			BLKIN_KEYVAL_STRING(this, ep, key, val);
	}
	void keyval(const char *key, int64_t val, const Endpoint *ep) const {
		if (valid())
			BLKIN_KEYVAL_INTEGER(this, ep, key, val);
	}

	// record timestamp annotations
	void event(const char *event) const {
		if (valid())
			BLKIN_TIMESTAMP(this, endpoint, event);
	}
	void event(const char *event, const Endpoint *ep) const {
		if (valid())
			BLKIN_TIMESTAMP(this, ep, event);
	}
};

}
#endif /* end of include guard: ZTRACER_H */

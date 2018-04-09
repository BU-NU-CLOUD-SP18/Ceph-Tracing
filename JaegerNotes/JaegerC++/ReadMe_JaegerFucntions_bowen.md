# Jaeger Functions
## Trasport
 - Exception(const std::string& what, int numFailed)
 - int numFailed() const { return _numFailed; } // from excepton
## UDP Transport Extense Transport
 - int append(const Span& span) override;
 - int flush() override;
 - void close() override { _client->close(); }
 - UDPTransport(const net::IPAddress& ip, int maxPacketSize);
## TracerFactory
 - MakeTracer(const char* configuration, std::string& errorMessage) const
 //make sure jaeger is build with yaml support
## Tracer
 - static std::shared_ptr<opentracing::Tracer>
    make(const std::string& serviceName, const Config& config)
    {
        return make(serviceName,
                    config,
                    std::shared_ptr<logging::Logger>(logging::nullLogger()));
    }
 - std::unique_ptr<opentracing::Span>
    StartSpanWithOptions(string_view operationName,
                         const opentracing::StartSpanOptions& options) const
        noexcept override;

    opentracing::expected<void> Inject(const opentracing::SpanContext& ctx,
                                       std::ostream& writer) const override
    {
        const auto* jaegerCtx = dynamic_cast<const SpanContext*>(&ctx);
        if (!jaegerCtx) {
            return opentracing::make_expected_from_error<void>(
                opentracing::invalid_span_context_error);
        }
        _binaryPropagator.inject(*jaegerCtx, writer);
        return opentracing::make_expected();
    }
 - opentracing::expected<void>
    Inject(const opentracing::SpanContext& ctx,
           const opentracing::TextMapWriter& writer) const override
    {
        const auto* jaegerCtx = dynamic_cast<const SpanContext*>(&ctx);
        if (!jaegerCtx) {
            return opentracing::make_expected_from_error<void>(
                opentracing::invalid_span_context_error);
        }
        _textPropagator.inject(*jaegerCtx, writer);
        return opentracing::make_expected();
    }
 - opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
    Extract(std::istream& reader) const override
    {
        const auto spanContext = _binaryPropagator.extract(reader);
        if (spanContext == SpanContext()) {
            return std::unique_ptr<opentracing::SpanContext>();
        }
        return std::unique_ptr<opentracing::SpanContext>(
            new SpanContext(spanContext));
    }
 - void Close() noexcept override
    {
        try {
            _reporter->close();
            _sampler->close();
            _restrictionManager->close();
        } catch (...) {
            utils::ErrorUtil::logError(*_logger,
                                       "Error occurred in Tracer::Close");
        }
    }
 - void reportSpan(const Span& span) const
    {
        _metrics->spansFinished().inc(1);
        if (span.context().isSampled()) {
            _reporter->report(span);
        }
    }
 - const std::string& serviceName() const { return _serviceName; }
 - const std::vector<Tag>& tags() const { return _tags; }
 - const baggage::BaggageSetter& baggageSetter() const
    {
        return _baggageSetter;
    }
## Tag
 - Tag(const std::pair<std::string,ValueArg> & tag_pair)
        : _key(tag_pair.first)
        , _value(tag_pair.second)
    {
    }
 - bool operator==(const Tag& rhs) const
    {
        return _key == rhs._key && _value == rhs._value;
    }
 - const std::string& key() const { return _key; }
 - const ValueType& value() const { return _value; }
 - thrift::Tag thrift() const
    {
        thrift::Tag tag;
        tag.__set_key(_key);
        ThriftVisitor visitor(tag);
        opentracing::util::apply_visitor(visitor, _value);
        return tag;
    }
## TraceID
 - TraceID(uint64_t high, uint64_t low)
        : _high(high)
        , _low(low)
    {
    }
 - bool isValid() const { return _high != 0 || _low != 0; }
 - void print(Stream& out) const
    {
        if (_high == 0) {
            out << std::hex << _low;
        }
        else {
            out << std::hex << _high << std::setw(16) << std::setfill('0')
                << std::hex << _low;
        }
    }
 - uint64_t high() const { return _high; }
 - uint64_t low() const { return _low; }
 - static TraceID fromStream(std::istream& in);
# Jaeger Functions
## Trasport
```C++
 - Exception(const std::string& what, int numFailed)
 - int numFailed() const { return _numFailed; } // from excepton
```
## UDP Transport Extense Transport
```C++
 - int append(const Span& span) override;
 - int flush() override;
 - void close() override { _client->close(); }
 - UDPTransport(const net::IPAddress& ip, int maxPacketSize);
```
### Usage
 ```C++
 -  const auto handle = testutils::TracerUtil::installGlobalTracer(); 
    const auto tracer =
    std::static_pointer_cast<const Tracer>(opentracing::Tracer::Global());
    UDPTransport sender(handle->_mockAgent->spanServerAddress(), 0); \\_mockAgent is a testing globle variable
    constexpr auto kNumMessages = 2000;
    const auto logger = logging::consoleLogger();
    for (auto i = 0; i < kNumMessages; ++i) {
        Span span(tracer);
        span.SetOperationName("test" + std::to_string(i));
    }
```
## TracerFactory
```C++
 - MakeTracer(const char* configuration, std::string& errorMessage) const
 //make sure jaeger is build with yaml support
```
 ### Usage
 ```C++
  - const char* config = R"(
  {
    "service_name": "test",
    "disabled": true,
    "sampler": {
      "type": "probabilistic",
      "param": 0.001
    },
    "reporter": {
      "queueSize": 100,
      "bufferFlushInterval": 10,
      "logSpans": false,
      "localAgentHostPort": "127.0.0.1:6831"
    },
    "headers": {
      "jaegerDebugHeader": "debug-id",
      "jaegerBaggageHeader": "baggage",
      "TraceContextHeaderName": "trace-id",
      "traceBaggageHeaderPrefix": "testctx-"
    },
    "baggage_restrictions": {
        "denyBaggageOnInitializationFailure": false,
        "hostPort": "127.0.0.1:5778",
        "refreshInterval": 60
    }
  })";
    TracerFactory tracerFactory;
    std::string errorMessage;
    auto tracerMaybe = tracerFactory.MakeTracer(config, errorMessage);
    // an exmaple of a valid config
```
## Tracer
```C++
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
```
### Usage Tracer
```C++
    const auto handle = testutils::TracerUtil::installGlobalTracer();
    const auto tracer =
        std::static_pointer_cast<Tracer>(opentracing::Tracer::Global());

    auto tagItr = std::find_if(
        std::begin(tracer->tags()),
        std::end(tracer->tags()),
        [](const Tag& tag) { return tag.key() == kJaegerClientVersionTagKey; });
    ASSERT_NE(std::end(tracer->tags()), tagItr);
    ASSERT_TRUE(tagItr->value().is<const char*>());
    ASSERT_EQ("C++-",
              static_cast<std::string>(tagItr->value().get<const char*>())
                  .substr(0, 4));

    opentracing::StartSpanOptions options;
    options.tags.push_back({ "tag-key", 1.23 });

    const FakeSpanContext fakeCtx;
    options.references.emplace_back(opentracing::SpanReferenceType::ChildOfRef,
                                    &fakeCtx);
    const SpanContext emptyCtx(TraceID(), 0, 0, 0, StrMap());
    options.references.emplace_back(opentracing::SpanReferenceType::ChildOfRef,
                                    &emptyCtx);
    const SpanContext parentCtx(
        TraceID(0xDEAD, 0xBEEF), 0xBEEF, 1234, 0, StrMap());
    options.references.emplace_back(opentracing::SpanReferenceType::ChildOfRef,
                                    &parentCtx);
    options.references.emplace_back(
        opentracing::SpanReferenceType::FollowsFromRef, &parentCtx);
    const SpanContext debugCtx(
        TraceID(),
        0,
        0,
        static_cast<unsigned char>(SpanContext::Flag::kSampled) |
            static_cast<unsigned char>(SpanContext::Flag::kDebug),
        StrMap({ { "debug-baggage-key", "debug-baggage-value" } }),
        "123");
    options.references.emplace_back(opentracing::SpanReferenceType::ChildOfRef,
                                    &debugCtx);

    const auto& tags = tracer->tags();
    auto itr =
        std::find_if(std::begin(tags), std::end(tags), [](const Tag& tag) {
            return tag.key() == kTracerIPTagKey;
        });
    ASSERT_NE(std::end(tags), itr);
    ASSERT_TRUE(itr->value().is<std::string>());
    ASSERT_EQ(net::IPAddress::v4(itr->value().get<std::string>()).host(),
              net::IPAddress::localIP(AF_INET).host());

    std::unique_ptr<Span> span(static_cast<Span*>(
        tracer->StartSpanWithOptions("test-operation", options).release()));
    ASSERT_TRUE(static_cast<bool>(span));
    ASSERT_EQ(static_cast<opentracing::Tracer*>(tracer.get()), &span->tracer());

    span->SetOperationName("test-set-operation");
    span->SetTag("tag-key", "tag-value");
    span->SetBaggageItem("test-baggage-item-key", "test-baggage-item-value");
    ASSERT_EQ("test-baggage-item-value",
              span->BaggageItem("test-baggage-item-key"));
    span->Log({ { "log-bool", true } });
    opentracing::FinishSpanOptions foptions;
    opentracing::LogRecord lr{};
    lr.fields = { {"options-log", "yep"} };
    foptions.log_records.push_back(std::move(lr));
    lr.timestamp = opentracing::SystemClock::now();
    span->FinishWithOptions(foptions);
    ASSERT_GE(Span::SteadyClock::now(),
              span->startTimeSteady() + span->duration());
    span->SetOperationName("test-set-operation-after-finish");
    ASSERT_EQ("test-set-operation", span->operationName());
    span->SetTag("tagged-after-finish-key", "tagged-after-finish-value");

    span.reset(static_cast<Span*>(
        tracer->StartSpanWithOptions("test-span-with-default-options", {})
            .release()));

    options.references.clear();
    options.references.emplace_back(
        opentracing::SpanReferenceType::FollowsFromRef, &parentCtx);
    span.reset(static_cast<Span*>(
        tracer->StartSpanWithOptions("test-span-with-default-options", options)
            .release()));

    options.references.clear();
    options.references.emplace_back(opentracing::SpanReferenceType::ChildOfRef,
                                    &debugCtx);
    span.reset(static_cast<Span*>(
        tracer->StartSpanWithOptions("test-span-with-debug-parent", options)
            .release()));

    options.references.clear();
    options.references.emplace_back(
        opentracing::SpanReferenceType::FollowsFromRef, &parentCtx);
    options.start_steady_timestamp = Tracer::SteadyClock::now();
    span.reset(static_cast<Span*>(
        tracer
            ->StartSpanWithOptions("test-span-with-default-system-timestamp",
                                   options)
            .release()));
    const auto calculatedSystemTime =
        static_cast<Tracer::SystemClock::time_point>(
            opentracing::convert_time_point<Tracer::SystemClock>(
                span->startTimeSteady()));
    ASSERT_GE(std::chrono::milliseconds(10),
              absTimeDiff<Tracer::SystemClock>(span->startTimeSystem(),
                                               calculatedSystemTime));

    options.start_system_timestamp = Tracer::SystemClock::now();
    span.reset(static_cast<Span*>(
        tracer
            ->StartSpanWithOptions("test-span-with-default-steady-timestamp",
                                   options)
            .release()));
    const auto calculatedSteadyTime =
        static_cast<Tracer::SteadyClock::time_point>(
            opentracing::convert_time_point<Tracer::SteadyClock>(
                span->startTimeSystem()));
    ASSERT_GE(std::chrono::milliseconds(10),
              absTimeDiff<Tracer::SteadyClock>(span->startTimeSteady(),
                                               calculatedSteadyTime));

    options.start_system_timestamp = Tracer::SystemClock::now();
    options.start_steady_timestamp = Tracer::SteadyClock::now();
    span.reset(static_cast<Span*>(
        tracer->StartSpanWithOptions("test-span-with-both-timestamps", options)
            .release()));
    ASSERT_EQ(options.start_system_timestamp, span->startTimeSystem());
    ASSERT_EQ(options.start_steady_timestamp, span->startTimeSteady());

    span.reset();

    opentracing::Tracer::InitGlobal(opentracing::MakeNoopTracer());

```
### Usage Propergation
```C++
    const auto handle = testutils::TracerUtil::installGlobalTracer();
    const auto tracer =
        std::static_pointer_cast<Tracer>(opentracing::Tracer::Global());
    const std::unique_ptr<Span> span(static_cast<Span*>(
        tracer->StartSpanWithOptions("test-inject", {}).release()));
    span->SetBaggageItem("test-baggage-item-key", "test-baggage-item-value");
    // Binary
    {
        std::stringstream ss;
        ASSERT_TRUE(static_cast<bool>(tracer->Inject(span->context(), ss)));
        auto result = tracer->Extract(ss);
        ASSERT_TRUE(static_cast<bool>(result));
        std::unique_ptr<const SpanContext> extractedCtx(
            static_cast<SpanContext*>(result->release()));
        ASSERT_TRUE(static_cast<bool>(extractedCtx));
        ASSERT_EQ(span->context(), *extractedCtx);
        FakeSpanContext fakeCtx;
        ss.clear();
        ss.str("");
        ASSERT_FALSE(static_cast<bool>(tracer->Inject(fakeCtx, ss)));
    }
    // Text map
    {
        StrMap textMap;
        WriterMock<opentracing::TextMapWriter> textWriter(textMap);
        ASSERT_TRUE(
            static_cast<bool>(tracer->Inject(span->context(), textWriter)));
        ASSERT_EQ(2, textMap.size());
        std::ostringstream oss;
        oss << span->context();
        ASSERT_EQ(oss.str(), textMap.at(kTraceContextHeaderName));
        ASSERT_EQ("test-baggage-item-value",
                  textMap.at(std::string(kTraceBaggageHeaderPrefix) +
                             "test-baggage-item-key"));
        ReaderMock<opentracing::TextMapReader> textReader(textMap);
        auto result = tracer->Extract(textReader);
        ASSERT_TRUE(static_cast<bool>(result));
        std::unique_ptr<const SpanContext> extractedCtx(
            static_cast<SpanContext*>(result->release()));
        ASSERT_TRUE(static_cast<bool>(extractedCtx));
        ASSERT_EQ(span->context(), *extractedCtx);
    }
    // HTTP map
    {
        StrMap headerMap;
        WriterMock<opentracing::HTTPHeadersWriter> headerWriter(headerMap);
        ASSERT_TRUE(
            static_cast<bool>(tracer->Inject(span->context(), headerWriter)));
        ASSERT_EQ(2, headerMap.size());
        std::ostringstream oss;
        oss << span->context();
        ASSERT_EQ(oss.str(), headerMap.at(kTraceContextHeaderName));
        ASSERT_EQ("test-baggage-item-value",
                  headerMap.at(std::string(kTraceBaggageHeaderPrefix) +
                               "test-baggage-item-key"));
        ReaderMock<opentracing::HTTPHeadersReader> headerReader(headerMap);
        auto result = tracer->Extract(headerReader);
        ASSERT_TRUE(static_cast<bool>(result));
        std::unique_ptr<const SpanContext> extractedCtx(
            static_cast<SpanContext*>(result->release()));
        ASSERT_TRUE(static_cast<bool>(extractedCtx));
        ASSERT_EQ(span->context(), *extractedCtx);
    }
```
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
### Usage
 -     const Tag tags[] = { { "testBool", true },
                         { "testDouble", 0.0 },
                         { "testInt64", 0LL },
                         { "testUint64", 0ULL },
                         { "testStr", std::string{ "test" } },
                         { "testNull", nullptr },
                         { "testCStr", "test" } };
// How to make tags, which you define freely
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
 ### Usage
  -     std::ostringstream oss;
    oss << TraceID(0, 10);
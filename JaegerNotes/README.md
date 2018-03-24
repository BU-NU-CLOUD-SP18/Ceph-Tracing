# Solve for Ceph(C++) and Jaeger(Go) langauge missmatch 

1. Use Jaeger C++ client
https://github.com/jaegertracing/cpp-client

based on thrift language translate
https://thrift.apache.org

2. Compile Go within C++

# Jaeger Tracing pkg
In applicaation, add pkg/log & pkg/tracing
'''go
	RunE: func(cmd *cobra.Command, args []string) error {
		logger := log.NewFactory(logger.With(zap.String("service", "route")))
		server := route.NewServer(
			net.JoinHostPort(routeOptions.serverInterface, strconv.Itoa(routeOptions.serverPort)),
			tracing.Init("route", metricsFactory.Namespace("route", nil), logger, jAgentHostPort),
			logger,
		)
		return server.Run()
	},
'''

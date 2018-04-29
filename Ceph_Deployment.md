
##Commands for Deploying Ceph and getting traces

ssh into VM1 where Ceph is Deployed
```console
ssh centos@128.31.25.229 -A
```

```console
cd ceph/build
```

Startup Ceph with desired configurations
```console
OSD=3 MON=3 RGW=1 ../src/vstart.sh -n
```

Stop Ceph so that the tracepoints can be enabled.:
```console
/home/centos/ceph/src/stop.sh
```

Start up an Lttng session and enable tracepoints
```console
lttng create blkin-test
```

```console
lttng enable-event --userspace zipkin:timestamp
```

```console
lttng enable-event --userspace zipkin:keyval
```

```console
lttng start
```

Startup Ceph again
```console
OSD=3 MON=3 RGW=1 ../src/vstart.sh -n
```

Go to a parallel terminal to run this
```console
ssh centos@128.31.25.229 -A
```

```console
~/ceph/build/bin/ceph-mgr -i x -c ~/ceph/build/ceph.conf
```

Now put something in using rados, check that it made it, get it back, and remove it
```console
./bin/rados mkpool test-blkin
```

```console
./bin/rados put test-object-1 ../src/vstart.sh --pool=test-blkin
```

```console
./bin/ceph osd map test-blkin test-object-1
```

```console
./bin/rados get test-object-1 ./vstart-copy.sh --pool=test-blkin
```

```console
md5sum vstart*
```

```console
./bin/rados rm test-object-1 --pool=test-blkin
```
Now stop Lttng session and see what was collected
```console
lttng stop
```

```console
lttng view
```

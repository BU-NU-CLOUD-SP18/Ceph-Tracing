
##Commands for Deploying Ceph and getting traces

```console
ssh centos@128.31.25.229 -A
```

```console
cd ceph/build
```

```console
OSD=3 MON=3 RGW=1 ../src/vstart.sh -n
```

```console
/home/centos/ceph/src/stop.sh
```

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

```console
OSD=3 MON=3 RGW=1 ../src/vstart.sh -n
```

```console
ssh centos@128.31.25.229 -A
```

```console
~/ceph/build/bin/ceph-mgr -i x -c ~/ceph/build/ceph.conf
```

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

```console
lttng stop
```

```console
lttng view
```

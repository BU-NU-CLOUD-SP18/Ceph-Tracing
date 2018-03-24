## Hot R.O.D Deployment
Follow the general guidiance from:
https://jaeger.readthedocs.io/en/latest/getting_started/#all-in-one-docker-image

# Dependencies:
- Docker
- Docker account
- Go
- glide
- github
- github ssh access key
- Go working directory ./ssh $GOPATH

# Comments for general guidiance 
Making sure Docker run is very smooth for me. 

download docker and make an docker account.

Login and run the code provided.
docker run -d -e COLLECTOR_ZIPKIN_HTTP_PORT=9411 -p5775:5775/udp -p6831:6831/udp -p6832:6832/udp \
  -p5778:5778 -p16686:16686 -p14268:14268 -p9411:9411 jaegertracing/all-in-one:latest

I would even try to argure that Opentracing is worth looking into.

As for runing HOT ROD:

mkdir -p $GOPATH/src/github.com/jaegertracing
cd $GOPATH/src/github.com/jaegertracing


git clone git@github.com:jaegertracing/jaeger.git jaeger

make sure you are using eacctly this line, else it would not work. For reasons I dont understand. 
git clone everything in an outside folder such as Documents then mv the jaeger folder in place at $GOPATH/src/github.com/jaegertracing
do not directly clone it in there. it does not work even with sudo.

make sure you have GO path
cd $GOPATH(this should change your directory to the parent of "go" folder)
 vim ~/.bashrc 
 source ~/.bashrc

make sure you are using git ssh which needs a key 
locally make a key and add to github under setting and ssh key.
pbcopy < ~/.ssh/id_rsa.pub
test it with 
 ssh -T git@github.com



cd jaeger
make install

Here you must have a go/src and have a GOPATH

cd examples/hotrod
go run ./main.go all

Time to pray!



             # May need to adjust later.
SPECROOT=/usr/opt/spectcl/5.1-004
SPECINC=$(SPECROOT)/include
SPECLIB= $(SPECROOT)/lib

CXXFLAGS=-I$(DAQINC) -std=c++11 -g -I$(SPECINC)
LDFLAGS=-L$(DAQLIB) -lTcp -ldataformat -ldaqio -lDataFlow -ldaqshm -lException -lrt\
	-Wl,-rpath="$(DAQLIB)" -g

EXECS=metertest rdoperf fragsrcperf socksend sockperf pipesend pipeperf \
	fragmaker ritemMaker runmaker bufferedoutperf checkevfiles evbfilecheck \
	evbfilecheck

all: $(EXECS)

metertest: MeterApi.o metertestmain.o
	$(CXX) -o metertest  $^ $(LDFLAGS)


rdoperf: rdoperf.o MeterApi.o
	$(CXX) -o rdoperf $^ $(LDFLAGS)


fragsrcperf: MeterApi.o fragsrcperf.o
	$(CXX) -o fragsrcperf $^  $(LDFLAGS)

socksend: socksend.o 
	$(CXX) -o socksend $^ $(LDFLAGS)


sockperf: sockperf.o MeterApi.o
	$(CXX) -o sockperf $^ $(LDFLAGS)


pipesend: pipesend.o
	$(CXX) -o pipesend $^

pipeperf: pipeperf.o MeterApi.o
	$(CXX) -o pipeperf $^ $(LDFLAGS)

fragmaker: fragmaker.o
	$(CXX) -o fragmaker $^ $(LDFLAGS)

ritemMaker: ritemMaker.o
	$(CXX) -o ritemMaker $^ $(LDFLAGS)

runmaker: runmaker.o
	$(CXX) -o runmaker $^ $(LDFLAGS)

bufferedoutperf: bufferedoutperf.o
	$(CXX) -o bufferedoutperf $^ $(LDFLAGS)


checkevfiles: checkevfiles.o
	$(CXX) -o checkevfiles $^ $(LDFLAGS)

evbfilecheck: evbfilecheck.o
	$(CXX) -o evbfilecheck $^ $(LDFLAGS) \
		-L$(SPECLIB) -lTclGrammerApp -Wl,-rpath="$(SPECLIB)"

clean:
	rm -f *.o
	rm -f $(EXECS)

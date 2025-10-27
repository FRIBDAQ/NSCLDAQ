.NOTPARALLEL:

MPI_TSRCS=CMPITransport_mpi.cpp CMPIFanoutTransport_mpi.cpp \
	CMPIFanoutClientTransport_mpi.cpp CRingItemMPIDataSource_mpi.cpp

MPI_THDRS=CMPITransport_mpi.h   CMPIFanoutTransport_mpi.h \
	CMPIFanoutClientTransport_mpi.h CRingItemMPIDataSource_mpi.h

MPI_SWTSRCS=CMPIClassifierApp_mpi.cpp CMPIClassifierApp_mpi.h \
	CMPIAppStrategy_mpi.cpp CMPIAppStrategy_mpi.h

MPI_TRSRCS=CMPITransformerApp_mpi.h CMPITransformerApp_mpi.cpp \
	CMPIAppStrategy_mpi.cpp CMPIAppStrategy_mpi.h

MPI_EDITORSRCS=CMPIBuiltRingItemEditorApp_mpi.h  \
	CMPIBuiltRingItemEditorApp_mpi.cpp 	CMPIAppStrategy_mpi.cpp CMPIAppStrategy_mpi.h

MPI_FULLEDITORSRCS=CMPIFullEventEditorApp_mpi.h CMPIFullEventEditorApp_mpi.cpp \
		CMPIAppStrategy_mpi.cpp CMPIAppStrategy_mpi.h

if MPI
MPI_FLAGS=$(shell $(MPICXX) -show|cut -d" " -f2-1000)

#  Define the transport sources and headers.

MPI_TRANSPORT_SRCS=$(MPI_TSRCS)
MPI_TRANSPORT_HDRS=$(MPI_THDRS)



# Similarly for SoftwareTrigger sources:
#  Define the sources for SoftwareTrigger

MPI_SoftwareTrigger_SRCS= $(MPI_SWTSRCS)

MPI_Transformer_SRCS=$(MPI_TRSRCS)

MPI_EventEditor_SRCS=$(MPI_EDITORSRCS)

MPI_FullEventEditor_SRCS= $(MPI_FULLEDITORSRCS)

# Below will cause warning from auto make but will work at least for now.
# It's needed to get the unittests to use MPICXX as the linker.

CXXLD=$(MPICXX)

endif

lib_LTLIBRARIES=libSwTrigger.la libTestClassifier.la libTestExtender.la

bin_PROGRAMS=SoftwareTrigger EventFilter Transformer EventEditor  FullEventEditor


libSwTrigger_la_SOURCES=CReceiver.cpp CTestTransport.cpp CTestTransport.h zhelpers.hpp \
	CZMQTransport.cpp CZMQClientTransport.cpp CZMQServerTransport.cpp \
	CSender.cpp CRingItemFileTransport.cpp CRingBufferTransport.cpp \
	CDispatcher.cpp CClientRegistry.cpp CZMQRouterTransport.cpp \
	CZMQDealerTransport.cpp CDataSourceElement.cpp CParallelWorker.cpp \
	CDataSinkElement.cpp CThreadedProcessingElement.cpp		\
	CRingItemTransportFactory.cpp CRingItemDispatcher.cpp		\
	CZMQRingItemDispatcher.cpp CRingItemBlockSourceElement.cpp \
	CRingItemZMQSourceElement.cpp        \
	CZMQRingItemSourceThread.cpp CZMQRingItemWorker.cpp	\
	CZMQRingItemThreadedWorker.cpp CZMQCommunicatorFactory.cpp \
	CCommunicatorFactoryMaker.cpp CRingItemMarkingWorker.cpp \
	CNullTransport.cpp CGather.cpp $(MPI_TRANSPORT_SRCS)	\
	CBuiltRingItemExtender.cpp CBuiltItemWorker.cpp CBuiltRingItemEditor.cpp

libSwTrigger_la_LDFLAGS=@top_builddir@/base/os/libdaqshm.la           \
	@top_builddir@/daq/format/libdataformat.la 		\
	@top_builddir@/base/thread/libdaqthreads.la		\
	@top_builddir@/base/uri/liburl.la			\
	@top_builddir@/base/dataflow/libDataFlow.la		\
	@LIBEXCEPTION_LDFLAGS@ @ZMQ_LDFLAGS@ @THREADLD_FLAGS@ $(MPI_FLAGS) \
	@UFMT_LDFLAGS@

libSwTrigger_la_CPPFLAGS=-I@top_srcdir@/daq/IO -I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/base/os -I@top_srcdir@/daq/format \
	-I@top_srcdir@/base/factories	\
	-I@top_srcdir@/base/thread -I@top_srcdir@/base/uri \
	-I@top_srcdir@/daq/eventbuilder \
	@LIBTCLPLUS_CFLAGS@ @THREADCXX_FLAGS@ $(MPI_FLAGS) @PIXIE_CPPFLAGS@ \
	@UFMT_CPPFLAGS@

include_HEADERS = CTransport.h CReceiver.h CZMQTransport.h CZMQClientTransport.h \
	CZMQServerTransport.h CSender.h CRingItemTransport.h CRingItemFileTransport.h \
	CRingBufferTransport.h CDispatcher.h CClientRegistry.h CZMQRouterTransport.h \
	CZMQDealerTransport.h CFanoutClientTransport.h CProcessingElement.h \
	CDataSourceElement.h CParallelWorker.h CDataSinkElement.h  \
	CThreadedProcessingElement.h CRingItemTransportFactory.h	\
	CRingItemDispatcher.h CZMQRingItemDispatcher.h CRingItemBlockSourceElement.h \
	CRingItemZMQSourceElement.h \
	CZMQRingItemSourceThread.h CZMQRingItemWorker.h CProcessor.h \
	CZMQRingItemThreadedWorker.h CCommunicatorFactory.h CZMQCommunicatorFactory.h \
	CCommunicatorFactoryMaker.h CRingItemMarkingWorker.h CNullTransport.h \
	CFanoutTransport.h CGather.h CBuiltRingItemExtender.h \
	CBuiltItemWorker.h CBuiltRingItemEditor.h		\
	CFullEventEditor.h					\
	$(MPI_TRANSPORT_HDRS)


SoftwareTrigger_SOURCES=swtriggerflags.c swtriggerflags.h SoftwareTrigger.cpp \
	CClassifierApp.h CClassifierApp.cpp \
	CZMQThreadedClassifierApp.cpp CZMQThreadedClassifierApp.h \
	CRingItemSorter.h CRingItemSorter.cpp CRingBlockDataSink.h \
	CRingBlockDataSink.cpp CZMQAppStrategy.h CZMQAppStrategy.cpp \
	$(MPI_SoftwareTrigger_SRCS)



SoftwareTrigger_DEPENDENCIES=libSwTrigger.la
SoftwareTrigger_CPPFLAGS=$(libSwTrigger_la_CPPFLAGS)
SoftwareTrigger_LDFLAGS=libSwTrigger.la $(libSwTrigger_la_LDFLAGS) -ldl

EventFilter_SOURCES=eventfilterflags.c eventfilterflags.h EventFilterMain.cpp \
	CEventFilterApp.cpp CEventFilterApp.h
EventFilter_DEPENDENCIES=libSwTrigger.la
EventFilter_CPPFLAGS=$(libSwTrigger_la_CPPFLAGS)
EventFilter_LDFLAGS= libSwTrigger.la $(libSwTrigger_la_LDFLAGS)



Transformer_SOURCES=Transformer.cpp swtriggerflags.c swtriggerflags.h \
	$(MPI_Transformer_SRCS)  \
	CTransformerApp.cpp CTransformerApp.h \
	CRingItemSorter.h CRingItemSorter.cpp CRingBlockDataSink.h \
        CRingBlockDataSink.cpp 				\
	CZMQTransformerApp.cpp CZMQTransformerApp.h CZMQAppStrategy.h  CZMQAppStrategy.cpp

Transformer_DEPENDENCIES=libSwTrigger.la
Transformer_CPPFLAGS=$(libSwTrigger_la_CPPFLAGS)
Transformer_LDFLAGS= libSwTrigger.la $(libSwTrigger_la_LDFLAGS) -ldl

EventEditor_SOURCES=swtriggerflags.h swtriggerflags.c \
	$(MPI_EventEditor_SRCS) \
	CTransformerApp.cpp CTransformerApp.h \
        CRingItemSorter.h CRingItemSorter.cpp CRingBlockDataSink.h \
        CRingBlockDataSink.cpp                          \
	EventEditor.cpp CBuiltRingItemEditorApp.h CBuiltRingItemEditorApp.cpp	\
	CZMQBuiltRingItemEditorApp.h CZMQBuiltRingItemEditorApp.cpp \
	CZMQAppStrategy.h  CZMQAppStrategy.cpp 

EventEditor_DEPENDENCIES=libSwTrigger.la
EventEditor_CPPFLAGS=$(libSwTrigger_la_CPPFLAGS)
EventEditor_LDFLAGS= libSwTrigger.la $(libSwTrigger_la_LDFLAGS) -ldl

FullEventEditor_SOURCES = swtriggerflags.h swtriggerflags.c CFullEventEditor.cpp \
	FullEventEditor.cpp	\
	CFullEventEditorApp.h CFullEventEditorApp.cpp \
	CZMQFullEventEditorApp.h CZMQFullEventEditorApp.cpp \
	CRingItemSorter.h CRingItemSorter.cpp \
	CRingBlockDataSink.h CRingBlockDataSink.cpp\
	$(MPI_FullEventEditor_SRCS) CZMQAppStrategy.h  CZMQAppStrategy.cpp

FullEventEditor_DEPENDENCIES=libSwTrigger.la
FullEventEditor_CPPFLAGS=$(libSwTrigger_la_CPPFLAGS)
FullEventEditor_LDFLAGS= libSwTrigger.la $(libSwTrigger_la_LDFLAGS) -ldl



libTestClassifier_la_SOURCES=TestClassifier.cpp
libTestClassifier_la_CPPFLAGS=$(libSwTrigger_la_CPPFLAGS)
libTestClassifier_la_LDFLAGS=libSwTrigger.la $(libSwTrigger_la_LDFLAGS)
libTestClassifier_la_DEPENDENCIES=libSwTrigger.la

libTestExtender_la_SOURCES=CTestExtender.cpp
libTestExtender_la_CPPFLAGS=$(libSwTrigger_la_CPPFLAGS)
libTestExtender_la_LDFLAGS=libSwTrigger.la $(libSwTrigger_la_LDFLAGS)
libTestExtender_la_DEPENDENCIES=libSwTrigger.la


BUILT_SOURCES=swtriggerflags.c swtriggerflags.h eventfilterflags.c eventfilterflags.h \
	transformer.c transformer.h eventeditor.h eventeditor.c \
	fulleventeditor.h fulleventeditor.c

eventeditor.c: eventeditor.h

eventeditor.h: eventeditor.ggo
	$(GENGETOPT) <@srcdir@/eventeditor.ggo -Feventeditor

fulleventeditor.c: fulleventeditor.h

fulleventeditor.h: fulleventeditor.ggo
	$(GENGETOPT) <@srcdir@/fulleventeditor.ggo  -Ffulleventeditor

swtriggerflags.c: swtriggerflags.h

swtriggerflags.h: swtriggerflags.ggo
	$(GENGETOPT) <@srcdir@/swtriggerflags.ggo -Fswtriggerflags

eventfilterflags.c: eventfilterflags.h

eventfilterflags.h: eventfilterflags.ggo
	$(GENGETOPT) <@srcdir@/eventfilterflags.ggo -Feventfilterflags

transformer.c: transformer.h

transformer.h: transformer.ggo
	$(GENGETOPT) <@srcdir@/transformer.ggo -Ftransformer

# Note regardless we ensure the MPI sources are distributed in the tarball.

EXTRA_DIST=swtriggerflags.ggo eventfilterflags.ggo ConceptsAndPrograms.xml \
	CommFramework.xml transformer.ggo eventeditor.ggo fulleventeditor.ggo \
	$(MPI_TSRCS) \
	$(MPI_THDRS)  $(MPI_SWTSRCS) $(MPI_EDITORSRCS) $(MPI_FULLEDITORSRCS)


####################################################################################
# unit testing:

noinst_PROGRAMS=unittests
unittests_SOURCES=TestRunner.cpp Asserts.h testxporttests.cpp zmqxporttests.cpp \
	zmqsvrxporttests.cpp zmqclitests.cpp zmqdealertests.cpp zmqrtrtests.cpp \
	zmqsendertests.cpp zmqreceivertests.cpp zmqdispattests.cpp	\
	ritemfxporttests.cpp rbufferxporttests.cpp ritemtransportfactoryTests.cpp	\
	dsrceltests.cpp	pworkertests.cpp sinktests.cpp tprocesstests.cpp \
	zmqworkertests.cpp	\
	$(libSwTrigger_la_SOURCES)


unittests_CPPFLAGS=$(libSwTrigger_la_CPPFLAGS) @CPPUNIT_CFLAGS@
unittests_LDFLAGS= @top_builddir@/daq/IO/libdaqio.la \
	$(libSwTrigger_la_LDFLAGS) @CPPUNIT_LDFLAGS@


UNITTESTS: unittests
	./unittests

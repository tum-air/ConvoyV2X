#
# OMNeT++/OMNEST Makefile for convoy-architecture
#
# This file was generated with the command:
#  opp_makemake -f --deep -KINET4_4_PROJ=../../inet4.4 -KSIMU5G_PROJ=../../simu5G -KVEINS_INET_PROJ=../../veins_inet -KVEINS_VEINS_5_2_PROJ=../../veins-veins-5.2 -DINET_IMPORT -DVEINS_INET_IMPORT -I$$\(INET4_4_PROJ\)/src -I$$\(SIMU5G_PROJ\)/src -I$$\(VEINS_INET_PROJ\)/src -I$$\(VEINS_VEINS_5_2_PROJ\)/src -I. -L$$\(INET4_4_PROJ\)/src -L$$\(SIMU5G_PROJ\)/src -L$$\(VEINS_INET_PROJ\)/src -lINET$$\(D\) -lsimu5g$$\(D\) -lveins_inet$$\(D\)
#

# Name of target to be created (-o option)
TARGET_DIR = .
TARGET_NAME = convoy-architecture$(D)
TARGET = $(TARGET_NAME)$(EXE_SUFFIX)
TARGET_IMPLIB = $(TARGET_NAME)$(IMPLIB_SUFFIX)
TARGET_IMPDEF = $(TARGET_NAME)$(IMPDEF_SUFFIX)
TARGET_FILES = $(TARGET_DIR)/$(TARGET)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(QTENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(QTENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = -I$(INET4_4_PROJ)/src -I$(SIMU5G_PROJ)/src -I$(VEINS_INET_PROJ)/src -I$(VEINS_VEINS_5_2_PROJ)/src -I.

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = $(LDFLAG_LIBPATH)$(INET4_4_PROJ)/src $(LDFLAG_LIBPATH)$(SIMU5G_PROJ)/src $(LDFLAG_LIBPATH)$(VEINS_INET_PROJ)/src  -lINET$(D) -lsimu5g$(D) -lveins_inet$(D)

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = src
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc, .msg and .sm files
OBJS = \
    $O/apps/ConvoyOrchestration.o \
    $O/apps/CooperativeManeuver.o \
    $O/apps/Detector.o \
    $O/apps/DtwinPublisher.o \
    $O/apps/DtwinSubscriber.o \
    $O/apps/Localizer.o \
    $O/apps/Maneuver.o \
    $O/apps/Publisher.o \
    $O/apps/Subscriber.o \
    $O/apps/Tracker.o \
    $O/controls/ConvoyControl.o \
    $O/controls/MembershipControl.o \
    $O/controls/MessagingControl.o \
    $O/controls/RoutingControl.o \
    $O/interfaces/BackendMessaging.o \
    $O/interfaces/MultipleInputSingleOutput.o \
    $O/nodes/BaseNode.o \
    $O/nodes/WrapperNode.o \
    $O/stores/DtwinStore.o \
    $O/stores/MemberStore.o \
    $O/stores/SubscriberStore.o \
    $O/world/synthetic/SyntheticWorld.o \
    $O/world/test/TestWorld.o \
    $O/messages/ConvoyControlFeedback_m.o \
    $O/messages/ConvoyControlService_m.o \
    $O/messages/CoopManeuver_m.o \
    $O/messages/DtwinSub_m.o \
    $O/messages/MemberStatus_m.o \
    $O/messages/ObjectList_m.o \
    $O/packets/MCSPacket_m.o

# Message files
MSGFILES = \
    messages/ConvoyControlFeedback.msg \
    messages/ConvoyControlService.msg \
    messages/CoopManeuver.msg \
    messages/DtwinSub.msg \
    messages/MemberStatus.msg \
    messages/ObjectList.msg \
    packets/MCSPacket.msg

# SM files
SMFILES =

# Other makefile variables (-K)
INET4_4_PROJ=../../inet4.4
SIMU5G_PROJ=../../simu5G
VEINS_INET_PROJ=../../veins_inet
VEINS_VEINS_5_2_PROJ=../../veins-veins-5.2

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
CONFIGFILE = $(shell opp_configfilepath)
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIBS = $(OPPMAIN_LIB) $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)
ifneq ($(PLATFORM),win32)
LIBS += -Wl,-rpath,$(abspath $(INET4_4_PROJ)/src) -Wl,-rpath,$(abspath $(SIMU5G_PROJ)/src) -Wl,-rpath,$(abspath $(VEINS_INET_PROJ)/src)
endif

COPTS = $(CFLAGS) $(IMPORT_DEFINES) -DINET_IMPORT -DVEINS_INET_IMPORT $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)
SMCOPTS =

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE (if COPTS has changed since last build)
# and make the object files depend on it
COPTS_FILE = $O/.last-copts
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
  $(shell $(MKPATH) "$O")
  $(file >$(COPTS_FILE),$(COPTS))
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
-include makefrag

#------------------------------------------------------------------------------

# Main target
all: $(TARGET_FILES)

$(TARGET_DIR)/% :: $O/%
	@mkdir -p $(TARGET_DIR)
	$(Q)$(LN) $< $@
ifeq ($(TOOLCHAIN_NAME),clang-msabi)
	-$(Q)-$(LN) $(<:%.dll=%.lib) $(@:%.dll=%.lib) 2>/dev/null

$O/$(TARGET_NAME).pdb: $O/$(TARGET)
endif

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile $(CONFIGFILE)
	@$(MKPATH) $O
	@echo Creating executable: $@
	$(Q)$(CXX) $(LDFLAGS) -o $O/$(TARGET) $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders smheaders

# disabling all implicit rules
.SUFFIXES :
.PRECIOUS : %_m.h %_m.cc

$O/%.o: %.cc $(COPTS_FILE) | msgheaders smheaders
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc -MD -MP -MF $O/$(basename $<)_m.h.d $(MSGCOPTS) $?

%_sm.cc %_sm.h: %.sm
	$(qecho) SMC: $<
	$(Q)$(SMC) -c++ -suffix cc $(SMCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

smheaders: $(SMFILES:.sm=_sm.h)

clean:
	$(qecho) Cleaning $(TARGET)
	$(Q)-rm -rf $O
	$(Q)-rm -f $(TARGET_FILES)
	$(Q)-rm -f $(call opp_rwildcard, . , *_m.cc *_m.h *_sm.cc *_sm.h)

cleanall:
	$(Q)$(CLEANALL_COMMAND)
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

help:
	@echo "$$HELP_SYNOPSYS"
	@echo "$$HELP_TARGETS"
	@echo "$$HELP_VARIABLES"
	@echo "$$HELP_EXAMPLES"

# include all dependencies
-include $(OBJS:%=%.d) $(MSGFILES:%.msg=$O/%_m.h.d)

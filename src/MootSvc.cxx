//$Header: /nfs/slac/g/glast/ground/cvs/CalibSvc/src/MootSvc/MootSvc.cxx,v 1.15 2008/05/30 23:30:14 jrb Exp $
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

#include <utility>   // for std::pair
// <vector>, <string> included by MootSvc.h

#include "MootSvc.h"
#include "mootCore/MoodConnection.h"
#include "mootCore/MootQuery.h"
#include "mootCore/FileDescrip.h"
#include "facilities/Util.h"
#include "facilities/commonUtilities.h"
#include "CalibData/Moot/MootData.h"

#include "Event/TopLevel/Event.h"
#include "LdfEvent/LsfMetaEvent.h" // needed for fsw keys, start_time
#include "LdfEvent/LsfCcsds.h"     // to retrieve scid

#include "GaudiKernel/ISvcLocator.h"


#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"


/// Instantiation of a static factory to create instances of this service
static SvcFactory<MootSvc>          MootSvc_factory;
const ISvcFactory& MootSvcFactory = MootSvc_factory;

namespace {

  unsigned lookupMootConfig(MOOT::MootQuery* q, unsigned scid,
                            unsigned startedAt) {

    MOOT::AcqSummaryInfo* pAcqInfo = 
      q->getAcqSummaryInfo(startedAt, scid);
    if (!pAcqInfo) return 0;
    std::string keyStr = pAcqInfo->getKey();
    if (keyStr.empty()) return 0;
    unsigned mootKey = facilities::Util::stringToUnsigned(keyStr);
    delete pAcqInfo;
    return mootKey;
  }

  CalibData::MootFilterCfg* makeMootFilterCfg(const MOOT::ConstitInfo& c) {
    using CalibData::MootFilterCfg;

    MootFilterCfg* f = 
      new MootFilterCfg(c.getKey(), c.getName(), c.getPkg(), 
                        c.getPkgVersion(), c.getFmxPath(), c.getSrcPath(),
                        c.getFswId(), c.getStatus(), c.getSchemaId(),
                        c.getSchemaVersionId(), c.getInstanceId());
    return f;
  }
}

MootSvc::MootSvc(const std::string& name, ISvcLocator* svc)
  : Service(name, svc), m_q(0), m_c(0), m_fixedConfig(false), m_mootParmCol(0)
{
  declareProperty("MootArchive", m_archive = std::string("") );
  declareProperty("UseEventKeys", m_useEventKeys = true);
  declareProperty("Verbose", m_verbose = false);
  declareProperty("scid" , m_scid = 0);  // by default, look it up
  declareProperty("StartTime", m_startTime = 0);
  declareProperty("MootConfigKey", m_mootConfigKey = 0);
  declareProperty("NoMoot", m_noMoot = false);
}

MootSvc::~MootSvc(){ }

StatusCode MootSvc::initialize()
{
  // Initialize base class
  if (m_mootParmCol) {    // already attempted initialization
    return (m_q) ? StatusCode::SUCCESS : StatusCode::FAILURE;
  }

  StatusCode sc = Service::initialize();
  if ( !sc.isSuccess() ) return sc;

  MsgStream log(msgSvc(), "MootSvc");

  log << MSG::INFO << "Specific initialization starting" << endreq;


  // Get properties from the JobOptionsSvc
  sc = setProperties();
  if ( !sc.isSuccess() ) {
    log << MSG::ERROR << "Could not set jobOptions properties" << endreq;
    return sc;
  }
  log << MSG::DEBUG << "Properties were read from jobOptions" << endreq;

  if (m_noMoot) {
    log << MSG::INFO << "Honoring request for no Moot" << endreq;
    return StatusCode::SUCCESS;
  }
  // Needed for access for fsw keys
  sc = serviceLocator()->service("EventDataSvc", m_eventSvc, true);
  if (sc .isFailure() ) {
    log << MSG::ERROR << "Unable to find EventDataSvc " << endreq;
    return sc;
  }

  /*
    Might want to add something here to force use of
    user-supplied LATC master key if so requested
  */

  // Make a MOOT::MootQuery instance
  m_q = makeConnection(m_verbose); 
  if (!m_q) return StatusCode::FAILURE;

  if (m_mootConfigKey) { // use this to set up everything else
    m_fixedConfig = true;
    m_lookUpStartTime = false;
    m_lookUpScid = false;     
    m_useEventKeys = false;
    m_hw = m_q->getMasterKey(m_mootConfigKey);
    if (!m_hw) {
      log << MSG::ERROR << "No LATC master associated with MOOT config "
          << m_mootConfigKey << endreq;
      // Normally would be a fatal error.  Let it go for testing
      // return StatusCode::FAILURE;
    }      
    return StatusCode::SUCCESS;
  }

  // If StartTime has been set to default (0) we'll look it up in event;
  // else use specified value.  Similarly for scid

  if (m_scid == 0) {
    m_lookUpScid = true;
  }
  if (m_startTime == 0) {
    m_lookUpStartTime = true;
    m_mootConfigKey = 0;
  }  else {
    m_lookUpStartTime = false;
    if (!m_lookUpScid) { // can look up config now
      m_mootConfigKey = lookupMootConfig(m_q, m_scid, m_startTime);
      if (!m_mootConfigKey) {
        log << MSG::ERROR << "No MOOT config for scid = " << m_scid
            << " and StartedAt = " << m_startTime << endreq;
        return StatusCode::FAILURE;
      }
      m_fixedConfig = true;
    }
  }
 
  // Arrange to be woken up once per event.  For now can't think of
  // anything handler needs to do except for 1st event
  IIncidentSvc* incSvc;
  sc = service("IncidentSvc", incSvc, true);
  if (sc.isSuccess() ) {
    int priority = 100;
    incSvc->addListener(this, "EndEvent", priority);
  }
  else {
    log << MSG::ERROR << "Unable to find IncidentSvc" << endreq;
    return sc;
  }

  // Get local info
  sc = getPrecincts();

  m_mootParmCol = new CalibData::MootParmCol(CalibData::MOOTSUBTYPE_latcParm);


  log << MSG::INFO << "Specific initialization completed" << endreq;
  return sc;
}


StatusCode MootSvc::finalize()
{
  MsgStream log(msgSvc(), "MootSvc");
  log << MSG::DEBUG << "Finalizing" << endreq;
  closeConnection();
    
  return Service::finalize();
}

void MootSvc::closeConnection() {
  if (m_q) {
    delete m_q;
    m_q = 0;
  }
  if (m_c) {
    delete m_c;
    m_c = 0;
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::INFO << "Closed connection to Moot db" << endreq;
  }
}

MOOT::MootQuery* MootSvc::makeConnection(bool verbose) {
  static unsigned nOpen = 4;     // keep MySQL connection for this many events
  if (m_q) return m_q;

  const std::string slacDefault("/afs/slac/glast/g/moot/archive-mood");

  std::string archEnv("$(MOOT_ARCHIVE)");
  std::string archEnvName("MOOT_ARCHIVE");

  bool envSet = false;

  // To make a connection, need definition for env. var MOOT_ARCHIVE
  //   If path has been supplied in job options, do setenv for it
  //     Special value of "*" means use default 
  //     (/afs/slac/g/glast/moot/archive-mood)
  //   else if $MOOT_ARCHIVE already has def, use that
  //   else try default value above 

  if (m_archive.size() == 0 ) {
    // Check to see if MOOT_ARCHIVE has a value.  
    int nExpand = facilities::Util::expandEnvVar(&archEnv);
    if (nExpand > 0) envSet = true;
    // If not, set m_archive to 
    else m_archive = slacDefault;
  }
  if (!envSet) {
    if (m_archive == std::string("*"))   { // use slac default
      m_archive = slacDefault;
    }
    facilities::commonUtilities::setEnvironment(archEnvName, m_archive, true);
  }
  // Maybe should have some logic here to get verbose connection
  // depending on debug level?
  try {
    m_c = new MOOT::MoodConnection(false, verbose);
    if (m_c) m_q = new MOOT::MootQuery(m_c);
  }
  catch (std::exception ex) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR 
             << "Could not open connection to MOOT dbs for archive " 
             << m_archive << endreq;
    return 0;
  }

  if (!m_q) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR 
             << "Could not open connection to MOOT dbs for archive " 
             << m_archive << endreq;
    return 0;
  }

  if (!verbose) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::INFO
             << "Successfully connect to MOOT dbs for archive "
             << m_archive << endreq;
  }
  m_countdown = nOpen;
  return m_q;
}


StatusCode MootSvc::getPrecincts() {

  std::vector<std::string> prNames;

  if (m_noMoot) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR << "MootSvc unavailable by job option request"
        << endreq;
    return StatusCode::FAILURE;
  }

  // Currently only called once at initialization time, when m_q is
  // known to be valid.  Might conceivably call if connection is re-established
  m_q->getPrecincts(prNames);
  std::vector<std::string> pclasses;
  pclasses.reserve(50);   // lots for precinct 'generic'
  m_parmPrecincts.reserve(60);

  int pclassIx = 0;
  for (unsigned ix = 0; ix < prNames.size(); ix++) {
    // Get parm classes 
    pclasses.clear();
    m_q->getParmClasses(pclasses, prNames[ix]);
    for (unsigned jx = 0; jx < pclasses.size(); jx++) {
      m_parmPrecincts.push_back(ParmPrecinct(pclasses[jx], prNames[ix]));
      pclassIx++;
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode MootSvc::queryInterface(const InterfaceID& riid, 
                                   void** ppvInterface)
{
  if ( IID_IMootSvc == riid )  {
    // With the highest priority return the specific interface of this service
    *ppvInterface = dynamic_cast<IMootSvc*> (this);
  } else  {
    // Interface is not directly available: try out a base class
    return Service::queryInterface(riid, ppvInterface);
  }
  addRef();
  return StatusCode::SUCCESS;
}

/// At EndEvent check if MySQL connection is open periodically.  
/// If so, close it.
void MootSvc::handle(const Incident& inc) {

  static unsigned nEvt = 0;

  if (inc.type() != "EndEvent" ) return;

  if (!nEvt)  { // update info from event stream  after very first event
    updateFswEvtInfo();
  }
  nEvt++;

  if (m_c) {
    if (m_countdown) m_countdown--;
    if  (!m_countdown) {
      closeConnection();
    }
  }
}


int MootSvc::latcParmIx(const std::string& parmClass) const {
  if (m_noMoot) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR << "MootSvc unavailable by job option request"
        << endreq;
    return -2;
  }


  if (!m_mootParmCol) return -2;
  const CalibData::MootParmVec& vec = m_mootParmCol->getMootParmVec();
  for (unsigned ix = 0; ix < vec.size(); ix++) {
    if (parmClass.compare(vec[ix].getClass()) == 0)
      return ix;
  }
  return -1;
}


// This function gets primary information (hw key, moot config)
// relating to configurations from the data.  It's called at the 
// end of the first event and also whenever a client requests 
// supported information types (LATC, filter)
StatusCode  MootSvc::updateFswEvtInfo() {
  // For now either we update from event or we don't update at all
  using namespace enums;

  //  if ((!m_useEventKeys) && (!m_lookUpStartTime) &&
  //  (!m_lookUpScid) ) return  StatusCode::SUCCESS;

  // If nothing comes from data, there is nothing for us to do 
  if (m_fixedConfig) return StatusCode::SUCCESS;

  MsgStream log(msgSvc(), "MootSvc");

  SmartDataPtr<Event::EventHeader> eventHeader(m_eventSvc, "/Event");
  if (!eventHeader) {
    log << "No Event header! " << endreq;
    return StatusCode::FAILURE;
  }

  SmartDataPtr<LsfEvent::MetaEvent> metaEvt(m_eventSvc, "/Event/MetaEvent");
  if (!metaEvt) {
    log << MSG::DEBUG << "No MetaEvent" << endreq;
    return StatusCode::FAILURE;
  }

  if (m_useEventKeys) {
    unsigned newMasterKey;
    m_hw = 0;
    switch (metaEvt->keyType()) {

    case Lsf::LpaKeys: {
      const lsfData::LpaKeys *lpaKeysTds = metaEvt->keys()->castToLpaKeys();
      newMasterKey = lpaKeysTds->LATC_master();
      break;
    }

    case Lsf::LciKeys: {
      const lsfData::LciKeys *lciKeysTds = metaEvt->keys()->castToLciKeys();
      newMasterKey = lciKeysTds->LATC_master();
      break;
    }
    default: 
      log << MSG::DEBUG << "Unknown key type in LsfEvent::MetaEvent"
          << endreq;
      return StatusCode::FAILURE;
    }
    if (newMasterKey)     m_hw = newMasterKey;
  }
  if (m_lookUpScid) {  
    SmartDataPtr<LsfEvent::LsfCcsds> ccsds(m_eventSvc, "/Event/Ccsds");
    if (!ccsds) {
      log << MSG::DEBUG << "No ccsds" << endreq;
      return StatusCode::FAILURE;
    }
    m_scid = ccsds->scid();
  }
  if (m_lookUpStartTime) {
    unsigned old = m_startTime;
    m_startTime = (metaEvt->run()).startTime();
    if (old != m_startTime) { // also refresh config
      if (!m_q) {
        m_q = makeConnection(m_verbose);
        if (!m_q) return StatusCode::FAILURE;
      }
      m_mootConfigKey = lookupMootConfig(m_q, m_scid, m_startTime);
      if (!m_mootConfigKey) {
        MsgStream log(msgSvc(), "MootSvc");

        log << MSG::ERROR << "No MOOT config for scid = " << m_scid
            << " and StartedAt = " << m_startTime << endreq;
        return StatusCode::FAILURE;
      }
    }
  }
  return StatusCode::SUCCESS;
}


/// Filter config routines
unsigned MootSvc::getActiveFilters(std::vector<CalibData::MootFilterCfg>&
                                   filters, unsigned acqMode) {
  using CalibData::MootFilterCfg;
  if (m_noMoot) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR << "MootSvc unavailable by job option request"
        << endreq;
    return StatusCode::FAILURE;
  }

  updateFswEvtInfo();
  if (!m_mootConfigKey) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR << "Cannot retrieve filters; no known MOOT config" 
        << endreq;
    return 0;
  }
  if (acqMode >= MOOT::LPA_MODE_count) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR << "Unknown acq. mode " << acqMode << endreq;
    return 0;
  }
  if (!m_q) {
    m_q = makeConnection(m_verbose);
    if (!m_q) return 0; //  StatusCode::FAILURE;
  }

  MOOT::LpaMode lpaMode = (MOOT::LpaMode) acqMode;

  std::vector<MOOT::ConstitInfo> constits;
  unsigned cnt = m_q->getActiveFilters(m_mootConfigKey, constits,
                                       lpaMode);

  for (unsigned ix = 0; ix < cnt; ix++) {
    MootFilterCfg* f = makeMootFilterCfg(constits[ix]);
    filters.push_back(*f);
    delete f;
  }
  return cnt;
}
unsigned MootSvc::getActiveFilters(std::vector<CalibData::MootFilterCfg>&
                                   filters) {
  using CalibData::MootFilterCfg;
  if (m_noMoot) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR << "MootSvc unavailable by job option request"
        << endreq;
    return StatusCode::FAILURE;
  }

  updateFswEvtInfo();
  if (!m_mootConfigKey) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR << "Cannot retrieve filters; no known MOOT config" 
        << endreq;
    return 0;
  }

  std::vector<MOOT::ConstitInfo> constits;
  if (!m_q) {
    m_q = makeConnection(m_verbose);
    if (!m_q) return 0; // StatusCode::FAILURE;
  }

  unsigned cnt = m_q->getActiveFilters(m_mootConfigKey, constits,
                                       MOOT::LPA_MODE_ALL);

  for (unsigned ix = 0; ix < cnt; ix++) {
    MootFilterCfg* f = makeMootFilterCfg(constits[ix]);
    filters.push_back(*f);
    delete f;
  }
  return cnt;
}

CalibData::MootFilterCfg* 
MootSvc::getActiveFilter(unsigned acqMode, unsigned handlerId,
                         std::string& handlerName) {
  if (m_noMoot) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR << "MootSvc unavailable by job option request"
        << endreq;
    return 0;
  }

  updateFswEvtInfo();
  if (!m_mootConfigKey) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR << "Cannot retrieve filters; no known MOOT config" 
        << endreq;
    return 0;
  }

  if (acqMode >= MOOT::LPA_MODE_count) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR << "Unknown acq. mode " << acqMode << endreq;
    return 0;
  }
  MOOT::LpaMode lpaMode = (MOOT::LpaMode) acqMode;

  if (!m_q) {
    m_q = makeConnection(m_verbose);
    if (!m_q) return  0;       //StatusCode::FAILURE;
  }

  const MOOT::ConstitInfo* pConstit = 
    m_q->getActiveFilter(m_mootConfigKey, lpaMode, handlerId, handlerName);
  if (!pConstit) return 0;

  CalibData::MootFilterCfg* f = makeMootFilterCfg(*pConstit);
  return f;
}


unsigned MootSvc::getHardwareKey()  {
  if (m_noMoot) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR << "MootSvc unavailable by job option request"
        << endreq;
    return 0;
  }
  updateFswEvtInfo();
  return m_hw;
}

std::string MootSvc::getMootParmPath(const std::string& cl, unsigned& hw) {
  const CalibData::MootParm* pParm = getMootParm(cl, hw);
  if (pParm) return pParm->getSrc();
  else return std::string("");
}

const CalibData::MootParm* MootSvc::getMootParm(const std::string& cl,
                                                   unsigned& hw) {
  if (m_noMoot) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR << "MootSvc unavailable by job option request"
        << endreq;
    return 0;
  }
  updateFswEvtInfo();
  hw = m_hw;
  if (hw != m_mootParmCol->fswKey() ) {
    StatusCode sc = updateMootParmCol();
    if (!sc.isSuccess() ) return 0;
  }
  // Find desired parm
  int ix = latcParmIx(cl);
  if (ix < 0) return 0;
  else return &m_mootParmCol->m_v[ix];
}

const CalibData::MootParmCol* MootSvc::getMootParmCol(unsigned& hw)  {

  if (m_noMoot) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR << "MootSvc unavailable by job option request"
        << endreq;
    return 0;
  }

  updateFswEvtInfo();
  if (!m_hw) return 0;
  hw = m_hw;
  if (hw != m_mootParmCol->fswKey() ) {
    StatusCode sc = updateMootParmCol();
    if (!sc.isSuccess() ) return 0;
  }
  return m_mootParmCol;
}

StatusCode MootSvc::updateMootParmCol( ) {

  using CalibData::MootParmCol;
  using CalibData::MootParm;
  using CalibData::MootParmVec;

  MootParmVec& v = m_mootParmCol->m_v;
  v.clear();

  m_mootParmCol->m_key = m_hw;

  std::vector<MOOT::ParmOffline> parmsOff;

  if (!m_q) {
    m_q = makeConnection(m_verbose);
    if (!m_q) return StatusCode::FAILURE;
  }

  if (!(m_q->getParmsFromMaster(m_hw, parmsOff)) ) {
    MsgStream log(msgSvc(), "MootSvc");
    log << MSG::ERROR 
             << "Unable to fetch parameter collection from Moot" << endreq;
    return StatusCode::FAILURE;
  }
   
  v.reserve(parmsOff.size() );
  std::vector<MOOT::ParmOffline>::const_iterator poff = parmsOff.begin();

  while (poff != parmsOff.end() ) {

    std::string path(m_archive);
    path += std::string("/") + poff->getSrc();
    
    // Find pclass in parm-precinct vector; prec = elt.second;
    std::string prec = findPrecinct(poff->getClass());
    
    MootParm p(poff->getKey(), poff->getClass(), poff->getClassFk(),
               path, poff->getSrcFmt(), poff->getStatus(),
               prec);
    v.push_back(p);

    poff++;
  }
  return StatusCode::SUCCESS;   // for now
}

std::string MootSvc::findPrecinct(const std::string& pclass) {
  for (unsigned ix = 0; ix < m_parmPrecincts.size(); ix++) {
    if (pclass.compare(m_parmPrecincts[ix].first) == 0)
      return m_parmPrecincts[ix].second;
  }
  return std::string("");
}



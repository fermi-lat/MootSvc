//$Header: /nfs/slac/g/glast/ground/cvs/MootSvc/MootSvc/IMootSvc.h,v 1.2 2008/06/09 19:47:07 jrb Exp $
#ifndef IMootSvc_h
#define IMootSvc_h 1

#include "GaudiKernel/IInterface.h"

// External constants
#include "GaudiKernel/ClassID.h"


// Forward declarations

namespace MOOT {
  class MootQuery;
}

namespace CalibData {
  class MootParm;
  class MootParmCol;
  class MootFilterCfg;
}

namespace MOOT {
  enum InfoSrc {
    INFOSRC_UNKNOWN = 0,
    INFOSRC_TDS = 1,
    INFOSRC_JO = 2
  };

  enum InfoItem {
    INFOITEM_MOOTCONFIGKEY = 0,
    INFOITEM_SCID = 1,
    INFOITEM_STARTEDAT = 2,
    INFOITEM_HWKEY = 3
  };
}

static const InterfaceID IID_IMootSvc ("IMootSvc", 1, 0);

/** @class IMootSvc 

    Abstract interface of a service for access to MOOT information.
    See also data class definitions in CalibData/Moot


    @author Joanne Bogart
*/

class IMootSvc : virtual public IInterface   {

public:
  // Re-implemented from IInterface
  static const InterfaceID& interfaceID() { return IID_IMootSvc; }

  /// Filter config routines
  /**
     Get info for all active filters (union over all modes)
   */
  virtual unsigned getActiveFilters(std::vector<CalibData::MootFilterCfg>&
                                    filters)=0;

  /**
     Get info for  active filters for mode @a acqMode
   */
  virtual unsigned getActiveFilters(std::vector<CalibData::MootFilterCfg>&
                                    filters, unsigned acqMode)=0;

  /**
     Get info and handler name for filter specified by mode and handler id
   */
  virtual 
  CalibData::MootFilterCfg* getActiveFilter(unsigned acqMode, 
                                            unsigned handlerId,
                                            std::string& handlerName)=0;

  /// See enum definitions for source (TDS or job options) and items
  /// (scid, etc.) in IMootSvc.h.
  /// Note if Moot config key is set in job options, all other items
  /// (scid, start time, hw key) are derived from this key, hence are
  ///  also considered to be set from job options.  
  /// Similarly if both start time and scid are set in job options, other
  /// items may be derived, so all are considered to be set from jo
  virtual MOOT::InfoSrc getInfoItemSrc(MOOT::InfoItem item)=0;

  /// Return Moot config key for current acquisition
  virtual unsigned getMootConfigKey()=0;

  /// Return absolute path for parameter source file of specified class.
  /// If none return empty string.
  virtual std::string getMootParmPath(const std::string& cl, 
                                      unsigned& hw)=0;

  /// Return parm containing GEM registers (not necessarily
  /// ROI though)
  virtual const CalibData::MootParm* getGemParm(unsigned& hw)=0;

  /// Return parm containing ROI registers (not necessarily
  /// other GEM registers though)
  virtual const CalibData::MootParm* getRoiParm(unsigned& hw)=0;

  /// Return MootParm structure for parameter source file of specified class.
  /// If none return blank structure.
  virtual const CalibData::MootParm* getMootParm(const std::string& cl, 
                                                 unsigned& hw)=0;

  // Return pointer to Moot parameter collection.  Also set output
  // arg. hw to current hw key
  virtual const CalibData::MootParmCol* getMootParmCol(unsigned& hw)=0;


  /// Return last LATC master key seen in data
  virtual unsigned getHardwareKey()=0;


  /// Return index in MootParmCol of specified class
  virtual int latcParmIx(const std::string& parmClass) const =0;


  /** Get handle for metadata access from mootCore.
      Clients, especially in production code, are STRONGLY DISCOURAGED 
      from using this connection directly since it is not guaranteed 
      to stick around. Instead call appropriate IMootSvc function to 
      fetch information.  
      getConnection() is only here in case IMootSvc doesn't keep up with
      developers' needs for Moot queries.
  */
  virtual MOOT::MootQuery* getConnection() const = 0;
};

#endif


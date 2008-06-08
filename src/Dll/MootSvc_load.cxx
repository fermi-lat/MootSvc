/** 
* @file MootSvc_load.cpp
* @brief This is needed for forcing the linker to load all components
* of the library.
*
*  $Header: $
*/

#include "GaudiKernel/DeclareFactoryEntries.h"

DECLARE_FACTORY_ENTRIES(MootSvc) {
  // Useful for now to fake event time  
  //  DECLARE_ALGORITHM(CalibEvtClock);

  //  DECLARE_SERVICE(CalibDataSvc);


  //  DECLARE_SERVICE(CalibMySQLCnvSvc);
  DECLARE_SERVICE(MootSvc);

} 

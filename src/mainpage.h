// Mainpage for doxygen

/** @mainpage package MootSvc
  @author Joanne Bogart
  @section intro Introduction
  This package contains classes and interfaces for Moot-related
  services.  Classes for some data returned by Moot services
  may be found in the CalibData package.

  MootSvc information may be looked up according to information in the
  event (hardware and software keys, acquisition start time) or these
  things can (also) be specified in job options.

  

  @section requirements requirements
  @include requirements
  <hr>
  @section notes release.notes
  release.notes
  <hr> 
  @section jobOptions jobOptions

  MootSvc has the following job option properties:

  <dl>
    <dt>MootArchive</dt>
       <dd>Default of empty string will cause production archive
           to be used</dd>
    <dt>MootConfigKey</dt>
       <dd>Defaults to 0; i.e., determine moot config from StartTime
           and scid.  If MootConfigKey is set to a non-zero value,
           that config will be used and values for job options UseEventKeys,
           StartTime and scid will be ignored.</dd>
    <dt>UseEventKeys</dt>
       <dd>Defaults to 'true'; that is, use event keys in data</dd>
    <dt>StartTime</dt>
        <dd>Default of 0 means use value in the data.   StartTime
         and scid may be used to look up corresponding MOOT configuration
         for an acquisition.</dd>
    <dt>scid</dt>
        <dd>Source id for data.  Defaults to 77, value for flight data.
         Special value of 0 means look it up in the data.
        </dd>
    <dt>noMoot</dt>
        <dd>Defaults to "false".  If true, MootSvc will not attempt
            to connect to Moot archive and db.  Subsequent calls
            to MootSvc functions will fail with error.
        </dd>
    <dt>Verbose</dt>
        <dd>Determines whether informational messages will be printed,
            including all details of MOOT database transactions.  
            Defaults to false.
         </dd>

  </dl> 


  @todo    Add new conversion service which can access MOOT offline
           calibration table.
 */


# ORDERING OF HEADERS IS SIGNIFICANT. Don't change this ordering.
# It is required to make the combined header ical.h properly.
set(COMBINEDHEADERSICAL
   ${TOPS}/kcal/libical/src/libical/icalversion.h
   ${TOPS}/kcal/libical/src/libical/icaltime.h
   ${TOPS}/kcal/libical/src/libical/icalduration.h
   ${TOPS}/kcal/libical/src/libical/icalperiod.h
   ${TOPS}/kcal/libical/src/libical/icalenums.h
   ${TOPS}/kcal/libical/src/libical/icaltypes.h
   ${TOPS}/kcal/libical/src/libical/icalrecur.h
   ${TOPS}/kcal/libical/src/libical/icalattach.h
   ${TOPB}/kcal/libical/icalderivedvalue.h
   ${TOPB}/kcal/libical/icalderivedparameter.h
   ${TOPS}/kcal/libical/src/libical/icalvalue.h
   ${TOPS}/kcal/libical/src/libical/icalparameter.h
   ${TOPB}/kcal/libical/icalderivedproperty.h
   ${TOPS}/kcal/libical/src/libical/icalproperty.h
   ${TOPS}/kcal/libical/src/libical/pvl.h
   ${TOPS}/kcal/libical/src/libical/icalarray.h
   ${TOPS}/kcal/libical/src/libical/icalcomponent.h
   ${TOPS}/kcal/libical/src/libical/icaltimezone.h
   ${TOPS}/kcal/libical/src/libical/icalparser.h
   ${TOPS}/kcal/libical/src/libical/icalmemory.h
   ${TOPS}/kcal/libical/src/libical/icalerror.h
   ${TOPS}/kcal/libical/src/libical/icalrestriction.h
   ${TOPS}/kcal/libical/src/libical/sspm.h
   ${TOPS}/kcal/libical/src/libical/icalmime.h
)

FILE(WRITE  ${KDE_FILE_H_FILE} "#ifdef __cplusplus\n")
FILE(APPEND ${KDE_FILE_H_FILE} "extern \"C\" {\n")
FILE(APPEND ${KDE_FILE_H_FILE} "#endif\n")

foreach (_current_FILE ${COMBINEDHEADERSICAL})
   FILE(READ ${_current_FILE} _contents)
   STRING(REGEX REPLACE "#include *\"ical.*\\.h\"" "" _contents "${_contents}")
   STRING(REGEX REPLACE "#include *\"config.*\\.h\"" "" _contents "${_contents}")
   STRING(REGEX REPLACE "#include *\"pvl\\.h\"" "" _contents "${_contents}" )
   FILE(APPEND ${KDE_FILE_H_FILE} "${_contents}")
endforeach (_current_FILE)

FILE(APPEND ${KDE_FILE_H_FILE} "\n")
FILE(APPEND ${KDE_FILE_H_FILE} "#ifdef __cplusplus\n")
FILE(APPEND ${KDE_FILE_H_FILE} "}\n")
FILE(APPEND ${KDE_FILE_H_FILE} "#endif\n")

MABE_BUILD_DIR="../../build" # Relative to THIS_DIR below

THIS_DIR=`pwd`
MABE_BUILD_DIR="${THIS_DIR}/${MABE_BUILD_DIR}"
LOCAL_BUILD_DIR="${THIS_DIR}/build"
PROJECTS=`ls "${THIS_DIR}/projects"`

# Allow user to skip compilation with -c
DO_COMPILE=1
while getopts ":cs:h" opt; do
  case $opt in
    c)
     DO_COMPILE=0
      ;;
    s)
      PROJECTS=${OPTARG}
      ;;
    h)
      echo "run_examples.sh runs the regression tests and compares them against previous values"
      echo "Optional command line arguments:"
      echo "    -h - Print this help screen"
      echo "    -c - Skip compilation and use local executables"
      echo "    -s TEST_NAME - Run only one test, TEST_NAME"
      exit 0
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
  esac
done


### Compile
if [ ${DO_COMPILE} -gt 0 ]
then
  mkdir -p "${LOCAL_BUILD_DIR}"
  cd "${MABE_BUILD_DIR}"
  ## Debug mode
  make clean
  make debug 
  # Make sure it compiled
  TERMINAL_ERROR_CODE=$?
  if ! test ${TERMINAL_ERROR_CODE} -eq 0;
  then
    echo "Error! Could not compile MABE in debug mode! (${expected_file})"
    exit 1
  fi
  # Move executable
  mv "${MABE_BUILD_DIR}/MABE" "${LOCAL_BUILD_DIR}/MABE_debug"
  ## Release mode
  make clean
  make 
  # Make sure it compiled
  TERMINAL_ERROR_CODE=$?
  if ! test ${TERMINAL_ERROR_CODE} -eq 0;
  then
    echo "Error! Could not compile MABE in release mode! (${expected_file})"
    exit 1
  fi
  # Move executable
  mv "${MABE_BUILD_DIR}/MABE" "${LOCAL_BUILD_DIR}/MABE"
  cd "${THIS_DIR}"
fi

FINAL_RETURN_CODE=0
ERROR_COUNTER=0
SUCCESSFUL_PROJECTS=""
UNSUCCESSFUL_PROJECTS=""
for NAME in ${PROJECTS}
do
  PROJ_SUCCESSFUL=1
  echo "${NAME}"
  # Setup directory structure
  PROJ_DIR="${THIS_DIR}/projects/${NAME}"
  OUTPUT_DIR="${PROJ_DIR}/output"
  FILE_DIR="${PROJ_DIR}/needed_files"
  EXPECTED_DIR="${PROJ_DIR}/expected"
  mkdir "${OUTPUT_DIR}" -p
  cd "${OUTPUT_DIR}"
  # Copy over mabe executable and all needed files
  cp "${LOCAL_BUILD_DIR}/MABE" ./
  cp "${LOCAL_BUILD_DIR}/MABE_debug" ./
  cp "${FILE_DIR}"/* ./
  # Run!
  env MABE_IS_REGEN=0 ./run_regression_test.sh
  TERMINAL_ERROR_CODE=$?
  if ! test ${TERMINAL_ERROR_CODE} -eq 0;
  then
    echo "Error encountered while running ${OUTPUT_DIR}/run_regression_test.sh"
    echo "Error code: ${TERMINAL_ERROR_CODE}"
    FINAL_RETURN_CODE=${TERMINAL_ERROR_CODE}
    ERROR_COUNTER=$((ERROR_COUNTER + 1))
    PROJ_SUCCESSFUL=0
  fi
  # Remove non-output files
  rm "${OUTPUT_DIR}/MABE"
  rm "${OUTPUT_DIR}/MABE_debug"
  NEEDED_FILES=`ls "${FILE_DIR}"`
  for needed_file in ${NEEDED_FILES}
  do
    rm "${OUTPUT_DIR}/${needed_file}"
  done
  # Check generated data against expected data
  EXPECTED_FILES=`ls "${EXPECTED_DIR}"`
  for expected_file in ${EXPECTED_FILES}
  do
    if ! [ -e "${OUTPUT_DIR}/${expected_file}" ]
    then
      echo "Error! Expected file not found in output! (${expected_file})"
      FINAL_RETURN_CODE=1
      ERROR_COUNTER=$((ERROR_COUNTER + 1))
      PROJ_SUCCESSFUL=0
    fi
    cmp ${expected_file}  "${EXPECTED_DIR}/${expected_file}"
    TERMINAL_ERROR_CODE=$?
    if ! test ${TERMINAL_ERROR_CODE} -eq 0;
    then
      echo "Error! File generated different terminal output! (${expected_file})"
      echo "Result of \"diff\" command (left is output, right is expected):"
      diff "${expected_file}" "${EXPECTED_DIR}/${expected_file}"
      echo "Error code: ${TERMINAL_ERROR_CODE}"
      FINAL_RETURN_CODE=${TERMINAL_ERROR_CODE}
      ERROR_COUNTER=$((ERROR_COUNTER + 1))
      PROJ_SUCCESSFUL=0
    fi
  done
  if [ ${PROJ_SUCCESSFUL} -gt 0 ]
  then
    SUCCESSFUL_PROJECTS="${SUCCESSFUL_PROJECTS} ${NAME}"
  else
    UNSUCCESSFUL_PROJECTS="${UNSUCCESSFUL_PROJECTS} ${NAME}"
  fi

  # Reset back to original directory
  cd "${THIS_DIR}" 
done

if [ ${FINAL_RETURN_CODE} -eq 0 ]
then
  echo ""
  echo "All examples successfully executed!"
else
  echo ""
  echo "${ERROR_COUNTER} total errors encountered!"
  echo "Successful projects:${SUCCESSFUL_PROJECTS}"
  echo "Unsuccessful projects:${UNSUCCESSFUL_PROJECTS}"
fi
exit ${FINAL_RETURN_CODE}

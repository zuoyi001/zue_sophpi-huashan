#!/bin/sh

#set -e # Exit immediately if a command exits with a non-zero status.
readonly ThirdpartyLibs=./thirdparty/image/share:./thirdparty/sqlite3/lib
readonly SdkLibs=.
export LD_LIBRARY_PATH=${SdkLibs}:${ThirdpartyLibs}:$LD_LIBRARY_PATH
readonly ModelConfig=./model/model.yaml
#
# Define Constants
#
readonly ERROR=0
readonly WARN=1
readonly INFO=2
readonly DEBUG=3

readonly RED='\033[0;31m'
readonly NC='\033[0m' # No Color

#
# Global Variables
#
VERBOSE=$INFO
#taskList=('time_test' 'track' 'search' 'irprocess' 'videoPrcoess' 'calibrate')

####################################
#     Utility Functions            #
####################################
#
# Write a message to the console if the verbose level is greater or equal
# to the parameter level.
#
# Param $1 - Logging level
# Param $2 - Text to write to console
#

log() {
    prefix="`date`"
    [ $1 -eq $ERROR ] && prefix="`date`: ERROR"
    [ $1 -eq $WARN ] && prefix="`date`: Warning"
    [ $1 -eq $DEBUG ] && prefix="`date`: Debug"
    if [ -z "${LOGFILE:-}" ]; then
        [ $1 -le $VERBOSE ] && echo "${prefix}: ${2}"
    else
        [ $1 -eq $ERROR ] && echo "${prefix}: ${2}" 1>&2
        [ $1 -le $VERBOSE ] && echo "${prefix}: ${2}" >> "${LOGFILE}"
    fi
}

####################################
#     Option Processing            #
####################################
#
# Print the usage message of this script.
#

print_usage() {
    set +x
    echo -e "Run one of sample programs."
    echo -e "Usage: ${0} [OPTION]..."
    echo
    echo "----------------------------------------"
    echo -e "./run.sh task -r=<run_option> --width=<width> --height=<height>"
    echo
    echo -e "tasks:"
    echo -e "time_test: report the time spent in modules"
    echo -e "search: retrieve face id  from gallery dataset through probe image"
    echo -e "irprocess: face anti-spoofing with RGB and IR images"
    echo -e "track: track faces in image sequence or yuv video"
    echo -e "videoProcess: video processing demonstration"
    echo -e "calibrate: camera calibration"
    echo
    echo -e "argument choices:"
    echo -e "-r, --run_option:\t bgr|yuv420|binary"
    echo -e "yuv420 and binary image format, need setting width and height, default 1080*1920\n"
    echo
    echo -e "optional argument:"
    echo -e "-i, --input_path: the input path"
    echo -e "-a, --auxi_path: the auxiliary ir image path"
    echo -e "-o, --output_path: the output path"
    echo -e "-w, --width: the width of input image"
    echo -e "-h, --height: the height of input image"
    echo -e "-b, --base_group: the base group input path"
    echo
    echo "----------------------------------------"
    echo

}

#
# Search a given string for a substring.  Returns 0 if the substring occurs within the string.
#
# Param $1 - The string to search for a given substring.
# Param $2 - The substring (needle)
#
contains() {
    haystack="$1"
    needle="$2"
    case "$haystack" in
        *$needle*)
            return 0
            ;;
    esac
    return 1
}

#
# Search a given string for a string.  Returns 0 if the string occurs within the string.
#
# Param $1 - The string to search for a given substring.
# Param $2 - The substring (needle)
#
contains_match() {
    haystack="$1"
    needle="$2"

    for thing in ${haystack}; do
        echo "${thing}"
        if [ "${thing}" = "${needle}" ]; then
            return 0
        fi
    done
    return 1
}

# Process command line options supporting both long and short options
# and allowing for options to have arguments.
#
# Note: This function does not take parameters and only works on the existing
#       global parameters and shell built-ins.
#
getOpts()
{
    log $INFO "getOpts $@"
    while [ $# -gt 0 ]; do
        case "$1" in
            -r | --run_option*)
                contains "$1" "="
                if [ $? -eq 0 ]; then
                    option='--run_option='
                    run_option=`echo $1 | cut -c${#option}-`
                else
                    run_option="$2"
                    shift
                fi
                ;;
            -i | --input_path*)
                contains "$1" "="
                if [ $? -eq 0 ]; then
                    option='--input_path='
                    input_path=`echo $1 | cut -c${#option}-`
                else
                    input_path="$2"
                    shift
                fi
                ;;
            -a | --auxi_path*)
                contains "$1" "="
                if [ $? -eq 0 ]; then
                    option='--auxi_path='
                    auxi_path=`echo $1 | cut -c${#option}-`
                else
                    auxi_path="$2"
                    shift
                fi
                ;;
            -b | --base_group*)
                contains "$1" "="
                if [ $? -eq 0 ]; then
                    option='--base_group='
                    base_group=`echo $1 | cut -c${#option}-`
                else
                    base_group="$2"
                    shift
                fi
                ;;

            -o | --output_path*)
                contains "$1" "="
                if [ $? -eq 0 ]; then
                    option='--output_path='
                    output_path=`echo $1 | cut -c${#option}-`
                else
                    output_path="$2"
                    shift
                fi
                ;;
            -w | --width*)
                contains "$1" "="
                if [ $? -eq 0 ]; then
                    option='--width='
                    width=`echo $1 | cut -c${#option}-`
                else
                    width="$2"
                    shift
                fi
                ;;
            -h | --height*)
                contains "$1" "="
                if [ $? -eq 0 ]; then
                    option='--height='
                    height=`echo $1 | cut -c${#option}-`
                else
                    height="$2"
                    shift
                fi
                ;;
            --help)
                print_usage
                exit 0
                ;;

            *)
                print_usage
                exit 1
                ;;
        esac
        shift
    done
}


#
# Processes the command line arguments.
# If usage is printed exits as 0, unknown parameter exits as 1.
#
process_command_line_arguments() {

    log $INFO "Command line: $0 $*"

    if [ -z "$1" ]; then
        print_usage
        exit -1
    else
        task=$1
        shift
    fi

    #if [[ ! " ${taskList[@]} " =~ " ${task} " ]]; then
    #    print_usage
    #    exit -1
    #fi
    getOpts $@


}

####################################
#     Main                         #
####################################

main() {
    process_command_line_arguments "$@"
    [ $? -ne 0 ] && exit 1

    # Process bgr image if not specified.
    [ -z "${run_option}" ] && run_option='bgr'
    if [ "${run_option}" = "yuv420" ] && { [ -z "${width}" ]  || [ -z "${height}" ] ;}; then
        echo "yuv420 input need configure width and height!"
        exit -1
    fi

    [ -z "${width}" ] && width=1080
    [ -z "${height}" ] && height=1920
    [ -z "${output_path}" ] && output_path=${PWD}/data/output
    if [ -d ${output_path} ]
    then
      rm -rf ${output_path}
    fi
    mkdir -p ${output_path}

    case ${task} in
        time_test)
        [ -z "${input_path}" ] && input_path=${PWD}/data/query
        log $INFO "./time_test ${ModelConfig} -r ${run_option} -i ${input_path} -o ${output_path} -w ${width} -h ${height}"
        cmd="./time_test ${ModelConfig} -r ${run_option} -i ${input_path} -o ${output_path} -w ${width} -h ${height}"
        echo -e "${RED} ${cmd} ${NC}"
        ${cmd}
        ;;
        search)
        [ -z "${input_path}" ] && input_path=${PWD}/data/query
        [ -z "${base_group}" ] && base_group=${PWD}/data/base
        log $INFO "./search ${ModelConfig} -r ${run_option} -i ${input_path} -b ${base_group} -w ${width} -h ${height}"
        cmd="./search ${ModelConfig} -r ${run_option} -i ${input_path} -b ${base_group} -w ${width} -h ${height}"
        echo -e "${RED} ${cmd} ${NC}"
        ${cmd}
        ;;
        irprocess)
        [ -z "${input_path}" ] && input_path=${PWD}/data/visible
        [ -z "${auxi_path}" ] && auxi_path=${PWD}/data/ir
        cmd="./irprocess ${ModelConfig} -r  ${run_option} -i ${input_path} -a ${auxi_path} -o ${output_path} -w ${width} -h ${height}"
        echo -e "${RED} ${cmd} ${NC}"
        ${cmd}
        ;;
        track)
        [ -z "${input_path}" ] && input_path=${PWD}/data/video
        cmd="./track  ${ModelConfig} -r ${run_option} -i ${input_path}  -o ${output_path} -w ${width} -h ${height}"
        echo -e "${RED} ${cmd} ${NC}"
        ${cmd}
        ;;
        videoProcess)
        [ -z "${input_path}" ] && input_path=${PWD}/data/video
        [ -z "${base_group}" ] && base_group=${PWD}/data/base
        cmd="./videoProcess ${ModelConfig} -r ${run_option} -i ${input_path} -b ${base_group}  -o ${output_path} -w ${width} -h ${height}"
        echo -e "${RED} ${cmd} ${NC}"
        ${cmd}
        ;;
        calibrate)
        [ -z "${input_path}" ] && input_path=${PWD}/data/calibrate/bgr.png
        [ -z "${auxi_path}" ] && auxi_path=${PWD}/data/calibrate/ir.png
        cmd="./calibrate ${ModelConfig} -r ${run_option} -i ${input_path} -a ${auxi_path} -o ${output_path} -w ${width} -h ${height}"
        echo -e "${RED} ${cmd} ${NC}"
        ${cmd}
        ;;
        *)
        print_usage
        echo "not support ${task} !!!"
        exit 1
        ;;
    esac
    chmod 777 -R ${output_path}
}
#main
main "$@"



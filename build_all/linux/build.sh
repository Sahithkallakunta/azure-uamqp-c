#!/bin/bash
#set -o pipefail
#

set -e

script_dir=$(cd "$(dirname "$0")" && pwd)
build_root=$(cd "${script_dir}/../.." && pwd)
run_unit_tests=ON
run_valgrind=

usage ()
{
    echo "build.sh [options]"
    echo "options"
    echo " -cl, --compileoption <value>  specify a compile option to be passed to gcc"
    echo "   Example: -cl -O1 -cl ..."
	echo "-rv, --run_valgrind will execute ctest with valgrind"
    echo ""
    exit 1
}

sync_dependencies ()
{
    sharedutildir=/usr/include/azuresharedutil
    # check to see if the file exists
    if [ ! -d "$sharedutildir" ]; 
    then
        read -p "The required Azure Shared Utility does not exist would you like to install the component  (y/n)?" input_var
        # download the shared file
        if [ "$input_var" == "y" ] || [ "$input_var" == "Y" ]
        then
            echo "preparing Azure Shared Utility"
        else
            exit 1
        fi
        
        rm -r -f ~/azure-c-shared-utility
        git clone https://github.com/Azure/azure-c-shared-utility.git ~/azure-c-shared-utility
        bash ~/azure-c-shared-utility/c/build_all/linux/build.sh -i
    fi
}

process_args ()
{
    save_next_arg=0
    extracloptions=" "

    for arg in $*
    do      
      if [ $save_next_arg == 1 ]
      then
        # save arg to pass to gcc
        extracloptions="$arg $extracloptions"
        save_next_arg=0
      else
          case "$arg" in
              "-cl" | "--compileoption" ) save_next_arg=1;;
			  "-rv" | "--run_valgrind" ) run_valgrind=1;;
              * ) usage;;
          esac
      fi
    done
}

process_args $*

rm -r -f ~/azure-amqp
mkdir ~/azure-amqp
pushd ~/azure-amqp
cmake -DcompileOption_C:STRING="$extracloptions" $build_root
make --jobs=$(nproc)
ctest -C "Debug" -V

if [[ $run_valgrind == 1 ]] ;
then
	ctest -j $(nproc) -D ExperimentalMemCheck -VV

fi

popd
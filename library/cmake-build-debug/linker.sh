unlink ./conv_data
unlink ./runfile

if [ $1 = "0" ]; then
    echo "runfile and conv_data unlinked"
else
    ln -s /Users/mari/work/MPPC/upgrade/cosmi/conv/cmake-build-debug/data ./conv_data
    ln -s /Users/mari/work/MPPC/upgrade/cosmi/rootfile ./runfile
    echo "runfile and conv_data linked"
fi
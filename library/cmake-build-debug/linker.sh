unlink ./conv_data
unlink ./runfile

if [ $1 = "0" ]; then
    echo "unlink ./conv_data"
    echo "unlink ./runfile"
else
    ln -s /Users/mari/work/MPPC/upgrade/cosmi/conv/cmake-build-debug/data ./conv_data
    ln -s /Users/mari/work/MPPC/upgrade/cosmi/rootfile ./runfile
    echo "ln -s /Users/mari/work/MPPC/upgrade/cosmi/conv/cmake-build-debug/data ./conv_data"
    echo "ln -s /Users/mari/work/MPPC/upgrade/cosmi/rootfile ./runfile"
fi
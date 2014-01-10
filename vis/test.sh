# 

awk '{ print $1,$2; }' ~/src/FullMonte/source/bluespec/output.exit.txt > stripped.txt
RenderDiff --file 10000:stripped.txt --mesh 2

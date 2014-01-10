while(<>)
{
    if (/Num Node:\s+([0-9]+)/){ print "Nodes $1\n"; last; }
}

while(<>)
{
    if(/Num Elem:\s+([0-9]+)/){ print "Tetras $1\n"; last; }
}
while(<>)
{
    if(/Num_Trig:\s+([0-9]+)/){ print "Faces $1\n"; last; }
}

while(<>)
{
    if(m/Num Photon:\s+([0-9]+)/){ print "Photons $1\n"; last; }
}

while(<>)
{
    if(/Number of threads:\s+([0-9]+)/){ print "Threads $1\n"; last; }
}

while(<>)
{
    if(/Num of Intersection:\s+([0-9]+)/){ print "Intersections $1\n"; last; }
}

while(<>)
{
    if(/Num of Step:\s+([0-9]+)/){ print "Steps $1\n"; last; }
}

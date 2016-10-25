function l=skipComments(fid)

while(!feof(fid))
	l = fgetl(fid);
	if (length(l) >= 1 && l(1) != '#' && !isspace(l(1)))
		break
	end
end

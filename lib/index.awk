NR==FNR {
	cache[$2] = $1
	next
}

{
	current[$2] = $1
	if(cache[$2] != $1) {
		print "changed:", $2
	}
}

END {
	for(file in current) {
		if(!(file in cache)) {
			print "added:", file
		}
	}
	for(file in cache) {
		if(!(file in current)) {
			print "removed:", file
		}
	}
}

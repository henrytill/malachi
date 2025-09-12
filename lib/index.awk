NR==FNR {
	cache[$2] = $1
	next
}

{
	current[$2] = $1
	if(cache[$2] != $1) {
		printf "op=changed path=\"%s\" hash=%s\n", $2, $1
	}
}

END {
	for(file in current) {
		if(!(file in cache)) {
			printf "op=added path=\"%s\" hash=%s\n", file, current[file]
		}
	}
	for(file in cache) {
		if(!(file in current)) {
			printf "op=removed path=\"%s\" hash=%s\n", file, cache[file]
		}
	}
}

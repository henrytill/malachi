{
	printf "added\x1F%s\x1F%s\x1F%s\x1F%s\x1E", repo, repohash, $2, $1
}

END {
	printf "\x1D"
}

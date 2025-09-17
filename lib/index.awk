{
	if ($5 == "A") op = "added"
	else if ($5 == "M") op = "changed"
	else if ($5 == "D") op = "removed"
	else next

	pathhash = ($5 == "D") ? $3 : $4
	printf "%s\x1F%s\x1F%s\x1F%s\x1F%s\x1E", op, repo, repohash, $6, pathhash
}

END {
	printf "\x1D"
}

{
	if ($5 == "A") op = "added"
	else if ($5 == "M") op = "changed"
	else if ($5 == "D") op = "removed"
	else next

	hash = ($5 == "D") ? $3 : $4
	printf "repo=\"%s\" op=%s path=\"%s\" hash=%s\n", repo, op, $6, hash
}

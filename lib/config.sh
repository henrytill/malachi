# -*- sh-shell: bash -*-

getcachedir() {
	if test -n "${XDG_CACHE_HOME:-}"
	then
		echo "${XDG_CACHE_HOME}/malachi"
	else
		echo "${HOME}/.cache/malachi"
	fi
}

getconfigdir() {
	if test -n "${XDG_CONFIG_HOME:-}"
	then
		echo "${XDG_CONFIG_HOME}/malachi"
	else
		echo "${HOME}/.config/malachi"
	fi
}

getdatadir() {
	if test -n "${XDG_DATA_HOME:-}"
	then
		echo "${XDG_DATA_HOME}/malachi"
	else
		echo "${HOME}/.local/share/malachi"
	fi
}

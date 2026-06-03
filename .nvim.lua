vim.lsp.config("slangd", {
	settings = {
		slang = {
			predefinedMacros = { "__SLANG__=1" },
			additionalSearchPaths = {
				"src/core/src",
				"src/app/src",
				"src/app/src/shader",
			},
			searchInAllWorkspaceDirectories = true,
		},
	},
})

--[[
	wrx engine demo app
]]

function wrx.conf(cfg)
	-- mandatory settings
	cfg.width = 640
	cfg.height = 360
	cfg.fps = 30
	cfg.name = "WRX-ENGINE"
	-- optional settings (these are the defaults unless specified)
	cfg.title = "WRX-ENGINE"
	cfg.idBits = 16
	cfg.settings = ""
	cfg.threads = 4
	cfg.server = false
end

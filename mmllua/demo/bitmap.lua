function main()
	mml.init()

	local window = Window.new("MML Lua demo", 640, 480, 1, { "resizable" })

	local now = 0
	local last = 0
	local frame_time = 0
	local fps_timer = 0
	local fps_string = "fps: 0"

	local logo = Bitmap:load("mml.png")
	local font = Font:load("res/jetbrainsmono.ttf", 14);

	local attribs = window:query()

	print(attribs.w .. ", " .. attribs.h .. ", " .. attribs.pixel_size)
	for i, v in ipairs(attribs.flags) do
		print(v)
	end

	local running = true
	while running do
		now = mml.get_time() / mml.get_frequency()
		frame_time = now - last;
		last = now

		fps_timer = fps_timer + frame_time
		if fps_timer > 1 then
			fps_timer = 0
			fps_string = "fps: " .. tostring(1 / frame_time)
			print(fps_string)
		end

		event = {}
		while window:next_event(event) do
			if event.type == "quit" then
				running = false
			elseif event.type == "key press" then
				print("press " .. event.key)
			elseif event.type == "key release" then
				print("release " .. event.key)
			elseif event.type == "mouse press" then
				print("mouse press " .. event.button)
			elseif event.type == "mouse release" then
				print("mouse release " .. event.button)
			end
		end

		local backbuffer = window:backbuffer()

		backbuffer:fill({ 0, 0, 0 })
		backbuffer:fill_rect({ 200, 200, 100, 100 }, { 255, 255, 255 })

		logo:copy(backbuffer, { 0, 0 }, { 0, 0, logo:width(), logo:height() })
		backbuffer:render_text(font, fps_string, { 0, 0 }, { 255, 255, 255 })

		window:update()
	end

	mml.deinit()
end

main()

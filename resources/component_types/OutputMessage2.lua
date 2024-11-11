OutputMessage2 = {
    message = "I am OutputMessage2",

    OnStart = function(self)
        print(self.entity:GetName() .. " OutputMessage2 Start" .. " : " .. self.message)
        -- Audio.Play(0, "title_music", true)
        -- self.timer = 1000
    end,

    OnUpdate = function(self)
        print(self.entity:GetName() .." OutputMessage2 Update " ..  " : Update On Frame " .. Application.GetFrame())     
        Text.Draw("OutputMessage2", 25, 25, "OpenSans-Regular", 16, 255, 255, 255, 255)
        Image.DrawUI("dog", 25, 90)
        -- Image.Draw("cat", 1, 1)
        -- Image.DrawEx("cat", -5, -5, 0, 1, 1, 0.5, 0.5, 255, 255, 255, 120, 0)
        -- Image.DrawPixel(400, 400, 255, 255, 255, 120)
        -- Image.DrawPixel(400, 405, 255, 0, 0, 255)
    end
}

Controller = {
    OnStart = function(self)
        print(self.entity:GetName() .. " Controller Start")
        -- Audio.Play(0, "title_music", true)
        -- self.count = 0
        self.x = 0
        self.y = 0
    end,

    OnUpdate = function(self)
        print(self.entity:GetName() .." Controller Update " ..  " : Update On Frame " .. Application.GetFrame())     
        -- Text.Draw("OutputMessage2", 25, 25, "OpenSans-Regular", 16, 255, 255, 255, 255)
        if (Input.GetKey("w"))
        then
            self.y = self.y - 1
        elseif (Input.GetKey("s"))
        then
            self.y = self.y + 1
        elseif (Input.GetKey("a"))
        then
            self.x = self.x - 1
        elseif (Input.GetKey("d"))
        then
            self.x = self.x + 1
        elseif (Input.GetKeyDown("tab"))
        then
            Entity.Instantiate("PlayerController")   
        end

        Image.Draw("cat", self.x, self.y)
        
          
        -- Image.DrawPixel(400, 400, 255, 255, 255, 120)
        -- Image.DrawPixel(400, 405, 255, 0, 0, 255)
    end
}

OutputMessage2 = {
    message = "I am OutputMessage2",

    OnStart = function(self)
        print(self.entity:GetName() .. " OutputMessage2 Start" .. " : " .. self.message)
    end,

    OnUpdate = function(self)
        print(self.entity:GetName() .." OutputMessage2 Update " ..  " : Update On Frame " .. Application.GetFrame())     
        Text.Draw("OutputMessage2", 25, 25, "OpenSans-Regular", 16, 255, 255, 255, 255)
    end
}

OutputMessage = {
    message = "I am OutputMessage",

    OnStart = function(self)
        print(self.entity:GetName() .. " OutputMessage Start" .. " : " .. self.message)
        self.entityRef = Entity.Instantiate("wolverine")    

        self.entityRef:GetComponent("OutputMessage2").count =  Application.GetFrame()
        
        self.timer = 0  
        Scene.DontDestroy(self.entityRef)
        
    end,

    OnUpdate = function(self)
        print(self.entity:GetName() .. " OutputMessage Update " .. " : Update On Frame " .. Application.GetFrame())  
        Camera.SetPosition(Camera.GetPositionX() - 0.1, Camera.GetPositionY() + 0.1)
        if self.timer == 100 then
            Scene.Load(Scene.GetCurrent())
        end
        self.timer = self.timer + 1
        -- Camera.SetZoom(Camera.GetZoom() - 0.01)
    end,

    OnLateUpdate = function(self)
        print(self.entity:GetName() .. " OutputMessage Late Update " .. " : Late Update")
        -- self.entity:RemoveComponent("OutputMessage2")
        -- self.entityRef = Entity.Instantiate("wolverine")    
    end
}

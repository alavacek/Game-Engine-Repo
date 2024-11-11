OutputMessage = {
    message = "I am OutputMessage",

    OnStart = function(self)
        print(self.entity:GetName() .. " OutputMessage Start" .. " : " .. self.message)
        self.entityRef = Entity.Instantiate("wolverine")       
        
    end,

    OnUpdate = function(self)
        print(self.entity:GetName() .. " OutputMessage Update " .. " : Update On Frame " .. Application.GetFrame())  
        -- Camera.SetPosition(Camera.GetPositionX() - 0.1, Camera.GetPositionY() + 0.1)
        -- Camera.SetZoom(Camera.GetZoom() - 0.01)
    end,

    OnLateUpdate = function(self)
        print(self.entity:GetName() .. " OutputMessage Late Update " .. " : Late Update")
        -- self.entity:RemoveComponent("OutputMessage2")
        -- self.entityRef = Entity.Instantiate("wolverine")    
    end
}

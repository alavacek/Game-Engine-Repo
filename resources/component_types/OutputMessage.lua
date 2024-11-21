OutputMessage = {
    message = "I am OutputMessage",

    OnStart = function(self)
        print(self.entity:GetName() .. " OutputMessage Start" .. " : " .. self.message)
        self.entityRef = Entity.Instantiate("PlayerController")   
        --Scene.DontDestroy(self.entityRef)

    end,

    OnUpdate = function(self)
        print(self.entity:GetName() .. " OutputMessage Update " .. " : Update On Frame " .. Application.GetFrame())  

    end,

    OnLateUpdate = function(self)
        print(self.entity:GetName() .. " OutputMessage Late Update " .. " : Late Update")
    end
}

Controller = {
    OnStart = function(self)
        -- print(self.entity:GetName() .. " Controller Start")
        -- Audio.Play(0, "title_music", true)
        -- self.count = 0
        self.x = 0
        self.y = 0
        self.rigidbody = self.entity:GetComponent("Rigidbody")
    end,

    OnUpdate = function(self)
        -- print(self.entity:GetName() .." Controller Update " ..  " : Update On Frame " .. Application.GetFrame())     
        -- Text.Draw("OutputMessage2", 25, 25, "OpenSans-Regular", 16, 255, 255, 255, 255)
        if (Input.GetKey("w"))
        then
           Camera.SetPosition(Camera.GetPositionX(), Camera.GetPositionY() + 0.05)      
        elseif (Input.GetKey("a"))
        then
            self.rigidbody:AddForce(Vector2(-10, 0))
        elseif (Input.GetKey("s"))
        then
            Camera.SetPosition(Camera.GetPositionX(), Camera.GetPositionY() - 0.05)
        elseif (Input.GetKey("d"))
        then
            self.rigidbody:AddForce(Vector2(10, 0))
        elseif (Input.GetKey("space"))
        then
            --self.rigidbody:AddForce(Vector2(0, -100))  
            -- local RBs = self.entity:GetComponents("Rigidbody")   
            self.entity:RemoveComponent("Rigidbody")
        elseif (Input.GetKeyDown("tab"))
        then
            --self.rigidbody:SetPosition(Vector2(-3, 0)) 
            self.entity:AddComponent("Rigidbody")

            local test = self.entity:GetComponent("Rigidbody"):GetUpDirection()  
            self.entity:RemoveComponent("Rigidbody")

            Entity.Destroy(self.entity)

        end

        Image.DrawEx("cat", self.rigidbody:GetPosition().x, self.rigidbody:GetPosition().y, self.rigidbody:GetRotation(), 1, 1, 0.5, 0.5, 255, 255, 255, 255, 0)
        
    end,

    OnCollisionEnter = function(self, collision)
        --print("Collision Enter with " .. collision.other:GetName())
    end, 

    OnCollisionExit = function(self, collision)
        --print("Collision Exit with " .. collision.other:GetName())
    end,

    OnTriggerEnter = function(self, collision)
        --print("Trigger Enter with " .. collision.other:GetName())
    end, 

    OnTriggerExit = function(self, collision)
        --print("Trigger Exit with " .. collision.other:GetName())
    end,

    OnDestroy = function(self)
        print("Destroy logic")
    end
}

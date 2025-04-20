A very simple demo to demonatrate Gimbal Lock in 3D game development

use key up and down rotate pitch (x)

use key left and right rotate yaw (y)

use key q and e to rotate roll (z)

press key r to reset all asix to 0

press y to set yaw rotate to 90 degree, and then you will find x and z rotation is the same.

because we use these code

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));// X轴
        model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));  // Y轴
        model = glm::rotate(model, glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f)); // Z轴


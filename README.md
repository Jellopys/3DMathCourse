# 3DMathCourse
State &amp; Context, Interpolation, Intersection, Collision, Noise

#
# State &amp; Context
When an actor with the HealthComponent recieves damage, it compares the damage instigators direction relative to the damage receivers direction and outputs the dot product of the two. Then check the dot products value against a "backstab tolerance" value to determine if the hit was a backstab or not. I then use that interaction to display a Backstab text and a backstab multiplier to the final damage.

#
# Interpolation
I created a camera interpolation for when i wanted to direct the camera to a certain position. Using interpolation smoothes the transition from the previous camera position to the new position. I also used an animation curve in the widget animation editor to make a "fancier" visibility change on the enemies health bar.

#
# Intersection
When i press a button (T and Tab in this case) I search all enemies to check how close they are to the player. If they are within a certain range, I check each enemies screen space position to filter out the one that is closest to the center of my screen. I then target it, which enables their healthbar widget visibility. If you press the target button again, it will search for the next best target and save the previously searched target in a list so the tab targeting doesnt flip flop between only the two closest.

#
# Collision
When an enemy takes damage, I check if the instigator has an interactable interface and if the instigator applies Pushback. In my case, I shoot an arrow that applies pushback with a value of 100. I offset the receivers position based on the pushback value and the instigators direction ( the arrows forward direction ). Further implementation of this would be to add interpolation curve on the pushback instead of one burst "AddActorWorldOffset". That is on the TODO list. :))

#
# Noise
I created a material that uses the Noise node to create a "fake" texture that moves through the noise depth with time to create a "pulsating" motion within the material. I then added some color hue shift to also change the color with time. I use this material on "Magical stuff".
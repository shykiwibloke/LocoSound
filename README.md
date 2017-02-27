# LocoSound
Raspi (on Jessie with SDL2) based realtime sound generator for Miniature Diesel Locomotive

Project uses sound samples from real locomotive and manipulates/mixes them to get a rich 16bit stereo sound experience
Supports:
- Idle + 8 notch (positions) Throttle
- 8 notch Dynamic Brake
- Locomotive Horn (can be sounded as long or as short as you like and still have the proper startup and trailing characteristics)
- Air Compressor (random)
- Full startup sequence (Bell, Fuel Rack charging, Prime Mover startup)
- System will respond realtime to control changes - so can be in the middle of reving up between notchs 2,3 and respond smoothly to the throttle being advanced to notch 7 or back to idle etc etc.

Designed to work with an on-board touch screen, which shows the overall status of the locomotive as fed to it by control computers (See Arduino-based separate project LocoControl - which manages the control panel and controls 3x Sabertooth Motor Controllers)

Sound routines are controlled using keyboard commands or USB serial from controller (see sep Arduino project):

T = Throttle Mode
D = Dynamic Brake Mode  
S = Prime Mover Start Sequence  
H = Horn start  
J = Horn stop  
0 = Idle  
1-8 = Notch 1 to 8 for both Dynamic and Throttle


System loads and uses small WAV file samples of each notch, horn and accel/decel etc. So easy to adapt to other types of locomotive
Fine tuning of app is possible via the config file - so you can have the acceleration sound very smooth, or have the revs drop back slightly as the load comes on before the motor can rev up as is common with 2nd gen EMD units.

Sound samples loaded with the project are for my New Zealand 'DFT' prototype which is an EMD unit built in London Ontario in the 1980's and later turbocharged. (Photos of construction on my website/facebook page.

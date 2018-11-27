::COPY .\Inc\*.*  %NsCore%\Inc\*.*
::COPY .\%1\*.lib %NsCore%\Lib\%1\*.lib
copy .\%1\*.dll "%NSVP_COMMON%\System\"
copy .\%1\*.pdb "%NSVP_COMMON%\System\"
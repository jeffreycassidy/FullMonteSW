####################################################################################################################################
## Optical-properties frame

proc makeOpticalFrame { parent mats legend } { 
    frame $parent.mats
    
    # display heading names			
    
    set collabels [list material mu_a mu_s g n {mu_s'} albedo]
    
    for { set j 0 } { $j < [llength $collabels] } { incr j } {
    	label $parent.mats.col$j -text [lindex $collabels $j]
    	grid $parent.mats.col$j -column $j -row 0
    }
    
    
    # display material names down the left
    
    for { set i 0 } { $i < [llength $legend] } { incr i } {
    	label $parent.mats.row$i -text [LegendEntry_label_get [lindex $legend $i]]
    	grid $parent.mats.row$i -column 0 -row [expr $i + 1]
    	
    }
    
    proc updateparam { parent i } {
    	global mu_s_prime$i
    	global albedo$i

        foreach p [list mu_a mu_s g n] {
            set $p [$parent.mats.$p$i get]
            $parent.mats.$p$i delete 0 end
            $parent.mats.$p$i insert 0 [eval "format \"%6.3f\" \$$p"]
        }

    	set mu_s_prime [format "%6.3f" [expr (1.0 - $g) * $mu_s]] 
    	set mu_s_prime$i $mu_s_prime
    	
    	if { $mu_a > 0 || $mu_s_prime > 0 } { set albedo$i [format "%6.3f" [expr $mu_s_prime/($mu_s_prime+$mu_a)]] } else {
    		set albedo$i "" }
    	return 1;
    }
    
    for { set i 0 } { $i < [llength $legend] } { incr i } {
    
    	set j 1
    	
    	set mat [lindex $mats $i]

    	# create entries for mu_s, mu_a, g, n
    	foreach param [list mu_a mu_s g n] {
            global $param$i
            set $param$i [format "%6.3f" [SimpleMaterial_${param}_get $mat]]
    		entry $parent.mats.$param$i -validate focusout -textvariable $param$i
            grid $parent.mats.$param$i -column $j -row [expr $i +1]
    		incr j
    	}
    	
    	foreach param [list mu_a mu_s g n] { $parent.mats.$param$i config -validatecommand "updateparam $parent $i" }
    	
    	updateparam $parent $i
    	
    	foreach param [list mu_s_prime albedo] {
    		label $parent.mats.$param$i -textvariable $param$i
    		grid $parent.mats.$param$i -column $j -row [expr $i+1]
    		incr j
    	}

    }
    
    pack .opt.mats
}


####################################################################################################################################
## VTK render window setup

proc doVTKWindow { } {

vtkRenderer ren
    ren SetUseDepthPeeling 1
    ren SetMaximumNumberOfPeels 100
    ren SetOcclusionRatio 0.001

vtkRenderWindow renwin
    renwin AddRenderer ren

vtkRenderWindowInteractor iren
    iren SetRenderWindow renwin
    [iren GetInteractorStyle] SetCurrentStyleToTrackballCamera

}



####################################################################################################################################
## Point-source placement frame

proc updatepostext { parent name component } {
    foreach c [list x y z] { set p$c [$parent.pos.$c get] }
	puts "Position ($component) updated to [format "%6.3f %6.3f %6.3f" $px $py $pz]"
    psr moveTo "$px $py $pz"
    renwin Render
	return 1
}

proc updatepostoval { parent name xn yn zn } {
    $parent.$name.x delete 0 END
    $parent.$name.y delete 0 END
    $parent.$name.z delete 0 END

    $parent.$name.x insert 0 [format "%6.3f" $xn]
    $parent.$name.y insert 0 [format "%6.3f" $yn]
    $parent.$name.z insert 0 [format "%6.3f" $zn]
}

proc makePointSourceFrame { parent name } {
    frame $parent.$name
    label $parent.$name.label -text "Isotropic point source position (x,y,z): "

    entry $parent.$name.x -validate focusout -validatecommand "updatepostext $parent $name x"
    $parent.$name.x insert 0 [format "%6.3f" 0.0]
    entry $parent.$name.y -validate focusout -validatecommand "updatepostext $parent $name y"
    $parent.$name.y insert 0 [format "%6.3f" 0.0]
    entry $parent.$name.z -validate focusout -validatecommand "updatepostext $parent $name z"
    $parent.$name.z insert 0 [format "%6.3f" 0.0]
    
    pack $parent.$name.label -side left
    pack $parent.$name.x -side left
    pack $parent.$name.y -side left
    pack $parent.$name.z -side left

    pack $parent.$name
}





####################################################################################################################################
## Mesh properties

proc makeMeshFrame { parent meshfn mesh meshrep scalevar } {
    frame $parent.mesh

    label $parent.mesh.meshlabel -text "Mesh file: "
    label $parent.mesh.meshfntext  -text $meshfn


    grid $parent.mesh.meshlabel    -row 0 -column 0
    grid $parent.mesh.meshfntext     -row 0 -column 1

    pack $parent.mesh

    frame $parent.scale

    label $parent.scale.sclabel -text "Scale: "
    radiobutton $parent.scale.mm -text "mm" -variable $scalevar -value "mm"
    radiobutton $parent.scale.cm -text "cm" -variable $scalevar -value "cm"

    pack $parent.scale.sclabel -side left
    pack $parent.scale.mm -side left
    pack $parent.scale.cm -side left
    pack $parent.scale

}




####################################################################################################################################
## Simulation properties

proc makeSimFrame { parent simcmd } {
    frame $parent.opts

    global phimin phimax
    
    set Npkt 1000000
    
    label $parent.opts.npktlabel -text "Packets: "
    entry $parent.opts.npkt -textvariable Npkt
    $parent.opts.npkt insert 0 "$Npkt"
    
    grid $parent.opts.npktlabel 	-column 0 	-row 0
    grid $parent.opts.npkt			-column 1	-row 0
    
    set Nth 8
    
    label $parent.opts.nthlabel -text "Threads: "
    entry $parent.opts.nth -textvariable Nth
    $parent.opts.nth insert 0 $Nth
    
    grid $parent.opts.nthlabel 	-column 0 	-row 1
    grid $parent.opts.nth			-column 1	-row 1

    set E 50
    label $parent.opts.elabel -text "Total energy (J): "
    entry $parent.opts.e -textvariable E
    $parent.opts.e insert 0 $E

    grid $parent.opts.elabel -column 0 -row 2
    grid $parent.opts.e -column 1 -row 2

    set phimin [format "%6.3f" $phimin]
    set phimax [format "%6.3f" $phimax]

    label $parent.opts.fluencerange -text "Fluence range to show (J/cm2):"
    entry $parent.opts.fluencemin -textvariable phimin -validate focusout -validatecommand "updatescale"
    entry $parent.opts.fluencemax -textvariable phimax -validate focusout -validatecommand "updatescale"

    grid $parent.opts.fluencerange -column 0 -row 3
    grid $parent.opts.fluencemin -column 1 -row 3
    grid $parent.opts.fluencemax -column 2 -row 3
    
    label $parent.opts.ofnlabel -text "Output file name: "
    entry $parent.opts.ofn -textvariable ofn
    
    grid $parent.opts.ofnlabel 	-column 0 	-row 4
    grid $parent.opts.ofn			-column 1	-row 4
    
    pack $parent.opts
    
    button $parent.go -command "$simcmd" -text "Go!"
    pack $parent.go
    
    label $parent.commentlabel -text "File comment text" -height 6
    pack $parent.commentlabel -side top
    
    text  $parent.comment 
    pack $parent.comment
    
    button $parent.save -text "Calculate & Save" -command {
        global mesh opt ofn
        set E [BLIKernel $mesh $opt [bsr getDescription] $Npkt]
        fluencerep Update $E 0
        set comm [$parent.comment get 1.0 end]
        write_fluence "$ofn" $mesh $E "$comm"
        renwin Render
    }
    pack $parent.save
    pack $parent
}


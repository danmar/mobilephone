
// Draw simple empty box with 2mm thickness
module drawbox(x,y,z) {
    difference() {
        cube([x+4, y+4, z]);
        translate([2,2,2]) {
            cube([x, y, z]);
        }
    }
}

// Draw two bars that holds potentiometer in place
module potholder() {
        translate([-7,-5,0]) {
            cube([2,10,5]);
        }
        translate([5,-5,0]) {
            cube([2,10,5]);
        }
}

// Draw box
module drawbox2(x,y,z,yoffset) {
    difference() {
        drawbox(x,y,z);
        translate([x/2,yoffset,1]) {
            cylinder(h=4,r=2,center=true, $fn=100);
        }
    }
    translate([x/2,yoffset,0]) {
        potholder();
    }
}

module drawmbed() {
    cube([65,35,10]);
}

module drawgsm() {
    cube([65,85,5]);
}

translate([-35,-50,-5]) {
    drawbox2(70,145,10,50);
}

translate([-30,-45,5]) {
    drawmbed();
}
translate([-30,10,5]) {
    drawgsm();
}


module roundedcylinder(radius, height, r2) {
    cylinder(h=height, r=radius);
    translate([0, 0, -r2])
        cylinder(h=r2, r=radius-r2);
    for (i=[0:359]) {
        translate([(radius-r2)*sin(i), (radius-r2)*cos(i), 0])
            sphere(r2);
    }
}

module roundedcube(x,y,z,r2) {
    cube([x, y, z]);
    translate([0,r2,-r2])
        cube([x,y-2*r2,r2]);
    translate([0,r2,0])
        rotate(90, [0,1,0]) cylinder(r=r2,h=x);
    translate([0,y-r2,0])
        rotate(90, [0,1,0]) cylinder(r=r2,h=x);
}

module telefonlur(length, width, height1, height2)
{
    roundedcube(length, width, height1, width/10);

    translate([0,width/2,0]) {
        roundedcylinder(width/2, height2, width/10);
    }

    translate([length,width/2,0]) {
        roundedcylinder(width/2, height2, width/10);
    }
}

telefonlur(120, 50, 5, 20);

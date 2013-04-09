module snurrskiva(diameter,height)
{
	radius = diameter / 2;

	difference() {
		cylinder(h = height, r = radius, center = true, $fn=100);

		for (i = [0:10]) {
			translate([0.75*radius*sin(i*30),0.75*radius*cos(i*30),0]) {
				cylinder(h=2*height, r=0.15*radius, center=true, $fn=100);
				cylinder(h=2*height, r1=0, r2=0.25*radius, center=true, $fn=100);
				cylinder(h=2*height, r1=0.25*radius, r2=0, center=true, $fn=100);
			}
		}
	}
}

snurrskiva(60,2);


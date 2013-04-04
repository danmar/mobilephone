
module snurrskiva()
{
	difference() {
		cylinder(h = 1, r1 = 40, r2 = 40, center = true);

		for (i = [0:10]) {
			translate([30*sin(i*30),30*cos(i*30),0]) {
				cylinder(h=2, r1=7, r2=7, center=true);
				cylinder(h=2, r1=0, r2=10, center=true);
				cylinder(h=2, r1=10, r2=0, center=true);
			}
		}
	}
}

snurrskiva();

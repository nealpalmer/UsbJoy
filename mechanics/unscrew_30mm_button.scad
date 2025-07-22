
id = 23;
od = 26;
wall = 2;
tab_wid = 15;
tab_hei = 6;

bottom_h = 1.5;

hei = 20;

module unscrew_30mm_button() 
{
	// outer strong pipe
 if (1)
 difference() {
 	cylinder(d=od+wall*2, h=hei, $fn=100);
	translate([0,0,-0.1])
 		cylinder(d=od, h=hei+1, $fn=100);
 }

	// 
 difference() {
	translate([0,0,bottom_h])
		cylinder(d=od, h=hei-bottom_h, $fn=100);

		// remove an overhang
	//translate([0,0,bottom_h-0.001])
 		//cylinder(d1=od, d2=1, h=od/2, $fn=100);
	translate([0,0,-0.1])
 		cylinder(d=id, h=hei*2, $fn=100);
	
	hull() {
		intersection() {
			//cylinder(d=od,h=tab_hei+bottom_h,$fn=100);
			translate([-50,-tab_wid/2,0])
				cube([100,tab_wid,tab_hei+bottom_h]);
		}
			// remove an overhang
		//cylinder(d=1,h=tab_hei+bottom_h+od/4);
	}
 }

 for (a=[0,180]) rotate(a=a,v=[0,0,1]) {
	difference() {
		union() {
 			translate([od/2,-10,hei-10])
				cube([12,10,10]);
 			translate([od/2-3,-10,hei-10])
				cube([4,4,10]);
		}
 		translate([od/2+wall+15/2,0,hei-10.1])
 			cylinder(d=15,h=10.2,$fn=30);
	}
 }
}
unscrew_30mm_button();


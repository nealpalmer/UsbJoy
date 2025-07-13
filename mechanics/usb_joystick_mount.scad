
	// usbjoystick board dimensions and hole locations
usbjoy_w = 43;
usbjoy_l = 58;
usbjoy_thick = 1.6; // 0.062" PCB thickness
x1 = 104.5; y1 = 113;
x2 = 120.5; y2 = 113;
x3 = 95; y3 = 62;
x4 = 130; y4 = 62;
l = 91; r = 134; lr = (l+r)/2;
b = 58; t = 116; bt = (b+t)/2;
//usbjoy_holes = [[-17, 23], [17,23], [-10,-23], [10,-23]];
usbjoy_holes = [[x1-lr, bt-y1], [x2-lr,bt-y2], [x3-lr,bt-y3], [x4-lr,bt-y4]];
usbjoy_x = 3;

BASE = 4;

button_d = 28;
button_nut_d = 36;

m3_d = 3.2; // really is 3.0mm
m3_wall = 2;
m3head_d = 6; // really is 5.5mm
m3head_len = 3.5; // really is 3mm
m3_len = 10;
m3locknut_hei = 4;

//usbjoy_standoff_height = 4;
usbjoy_standoff_height = m3_len+m3head_len - BASE - usbjoy_thick - m3locknut_hei;

USBC_W = 12.5;

wall = 4; // happens to be the added radius of the button_nut

module mount_flat() {
	difference() {
		union() {
				// ring around button
			cylinder(d=button_d+2*wall, h=10, $fn=60);

				// base for board
			hull () {
				cylinder(d=button_d, h=3, $fn=60);
				//translate([usbjoy_x,-usbjoy_l/10-button_d/2-wall,0]) {
					//cube([usbjoy_w,usbjoy_l/10,3]);
					//translate([usbjoy_w/2-12.5/2,usbjoy_l,5])
						//color("red") cube([12.5,36,7.5]); // usb-c cable end
				//}
				for (xy=[usbjoy_holes[2],usbjoy_holes[3]]) {
					translate([usbjoy_x+xy[0]+usbjoy_w/2,-usbjoy_l/2-button_nut_d/2+xy[1],0])
						cylinder(d=9,h=3,$fn=60);
				}
			}
			hull () {
				for (xy=usbjoy_holes) {
					translate([usbjoy_x+xy[0]+usbjoy_w/2,-usbjoy_l/2-button_nut_d/2+xy[1],0])
						cylinder(d=9,h=3,$fn=60);
				}
			}
			//translate([usbjoy_x,-usbjoy_l-button_d/2-wall,0])
				//cube([usbjoy_w,usbjoy_l,3]);

				// standoffs for board
			for (xy=usbjoy_holes) {
				translate([usbjoy_x+xy[0]+usbjoy_w/2,-usbjoy_l/2-button_nut_d/2+xy[1],0])
					cylinder(d1=9,d2=3+4,h=3+usbjoy_standoff_height,$fn=60);
			}
		}

			// hole for button
		cylinder(d=button_d, h=3*10, $fn=60, center=true);

			// holes for bolts
		for (xy=usbjoy_holes) {
			translate([3+xy[0]+usbjoy_w/2,-usbjoy_l/2-button_nut_d/2+xy[1],-0.1]) {
				cylinder(d=3.4,h=30,$fn=60);
				cylinder(d=m3head_d,h=m3head_len+0.1,$fn=60);
				translate([0,0,m3head_len+0.09])
					cylinder(d1=m3head_d,d2=3.0,h=0.75,$fn=60); // chamfer so that support isn't needed
			}
		}

			// hole for PIC programming header
		translate([3+usbjoy_w/2-15/2,-usbjoy_l/2-button_nut_d/2-25/2,-1])
			cube([15,25,10]);
	
	}
}

module mount_vert() {
	difference() {
		union() {
				// ring around button
			cylinder(d=button_d+2*wall, h=10, $fn=60);

				// nut diameter
			//translate([0,0,12])
			//#cylinder(d=button_nut_d,h=2,$fn=60);

				// base for board
			hull () {
				cylinder(d=button_d+2*wall, h=10, $fn=60);
				translate([button_nut_d/2-wall/2,-button_nut_d/2,0])
					cube([wall,button_nut_d,10]);
			}
			translate([button_nut_d/2-wall/2,-usbjoy_l/2,0]) {
				cube([wall,usbjoy_l,usbjoy_w]);
			}

				// standoffs for board
			intersection() {
				translate([button_nut_d/2-wall/2,-usbjoy_l/2,0]) {
					cube([wall+usbjoy_standoff_height,usbjoy_l,usbjoy_w]);
				}
				for (xy=usbjoy_holes) {
					translate([button_nut_d/2+wall/2-0.01,xy[1],usbjoy_w/2+xy[0]])
						rotate(a=90,v=[0,1,0])
							cylinder(d1=m3_d+2*m3_wall+usbjoy_standoff_height,d2=m3_d+2*m3_wall,h=0*wall+usbjoy_standoff_height,$fn=60);
				}
			}
		}

			// hole for button
		cylinder(d=button_d, h=3*10, $fn=60, center=true);
			// hole for button's nut
		translate([0,0,10+0.01])
			cylinder(d=button_nut_d+1, h=10, $fn=60);
		translate([0,0,20-0.01])
			cylinder(d1=button_nut_d+1, d2=button_nut_d-wall, h=20, $fn=60);

			// holes for bolts
		for (xy=usbjoy_holes) {
			translate([button_nut_d/2,xy[1],usbjoy_w/2+xy[0]])
				rotate(a=90,v=[0,1,0])
					cylinder(d=3.5,h=wall+1.5+10,$fn=60,center=true);
		}
	
	}
}

if (1)
translate([100,0,0])
mount_flat();

if (1)
mount_vert();


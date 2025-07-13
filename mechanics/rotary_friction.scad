INCH = 25.4;

module encoder() {
	difference() {
		union() {
				// base of encoder
			translate([0,0,-1-0.115*INCH])
				cylinder(d=38,h=1,$fn=100);

				// rim of encoder
			translate([0,0,-0.115*INCH])
				cylinder(d=20,h=5,$fn=100);

				// shaft of encoder
			translate([0,0,-0.115*INCH+5])
				cylinder(d=6,h=13,$fn=100);

				// flange
			translate([0,0,-0.115*INCH+5+13-2])
				cylinder(d=22,h=2,$fn=100);
		}

			// M3 holes
		for (a=[0,120,240]) {
			rotate(a=a,v=[0,0,1])
				translate([0,28/2,-10])
					cylinder(d=3,h=20,$fn=30);
			rotate(a=a-30,v=[0,0,1])
				translate([0,30/2,-10])
					cylinder(d=3,h=20,$fn=30);
		}
	}
}

M3_D = 3.1;
HOLE_DIST = 30*cos(30);
WALL=6;
BASE = 6;
SPRING_THICK = 1;
SPRING_HEI = 3;

module springy(index) {
	difference() {
		union() {
				// base
			hull () {
				translate([(index==0) ? HOLE_DIST/3+5 : 0,0,0])
					cube([(index==0) ? HOLE_DIST*2/3-5 : HOLE_DIST,BASE/2,M3_D+2*WALL]);
				translate([(M3_D+2*WALL)/2-2,0*BASE/2,0])
					cube([HOLE_DIST*2/3,BASE/2,(M3_D+2*WALL)/2*0+1]);
			}
			hull () {
				translate([0,BASE/2,0])
					cube([HOLE_DIST*2/3-5,BASE/2,M3_D+2*WALL]);
				translate([HOLE_DIST/3+3,BASE/2,0])
					cube([HOLE_DIST*2/3-(M3_D+2*WALL)/2-1,BASE/2,(M3_D+2*WALL)/2*0+1]);
			}

				// 2 mounting holes
			translate([0,(index==0) ? BASE/2 : 0,1.5+WALL])
				rotate(a=-90,v=[1,0,0])
					cylinder(d=M3_D+2*WALL,h=BASE/((index==0) ? 2 : 1),$fn=30);
			translate([HOLE_DIST,0,1.5+WALL])
				rotate(a=-90,v=[1,0,0])
					cylinder(d=M3_D+2*WALL,h=BASE/2,$fn=30);
		}
			// 2 mounting holes
		for (x=[0,HOLE_DIST])
			translate([x,0,1.5+WALL])
				rotate(a=90,v=[1,0,0])
					cylinder(d=3.1,h=100,center=true,$fn=30);

			// center flange cut
		translate([HOLE_DIST/2,-1,HOLE_DIST*sin(30)/cos(30)])
			rotate(a=90,v=[-1,0,0])
				cylinder(d=22,h=5-0.115*INCH+1,$fn=100);
		translate([HOLE_DIST/2,-1,HOLE_DIST*sin(30)/cos(30)])
			rotate(a=90,v=[-1,0,0])
				cylinder(d=15,h=50-0.115*INCH+1,$fn=100);
	}
	// 2 springs?
	hull() {
		translate([0,BASE,0])
			cylinder(d=SPRING_THICK,h=SPRING_HEI,$fn=20);
		translate([HOLE_DIST/2,15.5,0])
			cylinder(d=SPRING_THICK,h=SPRING_HEI,$fn=20);
	}
}


color("grey")
encoder();


translate([50,0,0])
springy(0);

translate([100,0,0])
springy(120);


for (a=[0,120])
rotate(a=a,v=[0,0,1])
translate([0,15,0])
	rotate(a=90,v=[1,0,0])
		rotate(a=60,v=[0,-1,0])
			translate([0,0,-WALL-M3_D/2])
				springy(a);

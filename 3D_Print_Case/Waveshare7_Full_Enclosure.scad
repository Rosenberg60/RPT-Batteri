// =============================================================================
// PARAMETRIC WALL-MOUNT ENCLOSURE FOR WAVESHARE ESP32-S3-TOUCH-LCD-7 (REV 1.2)
// Designed for Bambu Lab 3D Printers (Bambu Studio / Bambu P1S / X1C / A1 / H2D)
// -----------------------------------------------------------------------------
// Features:
// 1. Flush Sunken Front Bezel (glass is 100% plant / flush with front face)
// 2. Snap-Fit tool-free closure with 2x pry-slots for easy disassembly
// 3. Integrated 18650 Li-ion / LiPo snap-in battery cradle in bottom of case
// 4. Port cutouts (2x USB-C + MicroSD TF card) on both right and left sides
// 5. Wall-mount keyhole slots on back (horizontal spacing 120mm)
// 6. Rear and bottom cable feedthroughs + cooling ventilation slots
// 7. 4x M3 mounting boss posts inside front bezel matching Waveshare factory studs
// =============================================================================

/* [Render Mode] */
part = "both"; // [both, front, back]
exploded_distance = 45;

/* [Display Specifications (Waveshare 7.0" Touch)] */
glass_w        = 192.96;
glass_h        = 110.76;
glass_r        = 7.88;
glass_pocket_d = 2.0;     // Recess depth so touch glass is 100% flush with bezel face
active_w       = 155.0;
active_h       = 87.0;
glass_clearance= 0.45;    // Fit clearance per side (0.9mm total W/H)

// 4x Waveshare Factory M3 brass standoffs on back of LCD (from official drawing)
// Horizontal distance 126.20mm, vertical 65.65mm, shifted by -1.53mm in X relative to glass
ws_standoff_x1 = -64.63;
ws_standoff_x2 =  61.57;
ws_standoff_y1 =  33.80;
ws_standoff_y2 = -31.85;

// Front bezel internal clamp boss positions (anchored to top & bottom perimeter walls)
bezel_boss_y_top =  53.5;
bezel_boss_y_bot = -53.5;
bezel_boss_x_dist = 100.0; // Spacing: X = +/- 50mm

// LCD metal chassis body size behind the glass
lcd_body_w     = 166.5;   // Clearance for 164.9mm LCD chassis
lcd_body_h     = 101.5;   // Clearance for 100.0mm LCD chassis

/* [18650 Battery Cradle Specs] */
bat_d             = 18.6;   // 18650 cell diameter (+ clearance)
bat_r             = bat_d / 2;
bat_len           = 66.5;   // Length clearance for 18650 cell
bat_y_pos         = -38.0;  // Position in bottom half of back case

/* [Enclosure Dimensions] */
wall           = 2.4;
outer_w        = 228.0;   // 228mm (+20mm wider for plenty of internal USB plug room)
outer_h        = 126.0;
corner_r       = 9.5;
total_depth    = 32.0;    // 32mm total depth (plenty of room for 18650 + CAN wiring)
front_depth    = 8.0;     // Front bezel rim depth
back_depth     = total_depth - front_depth; // 24.0mm
lip_height     = 4.5;
snap_depth     = 0.75;

$fn = 48;

module rounded_rect(w, h, r) {
    hull() {
        translate([-w/2 + r, -h/2 + r]) circle(r=r);
        translate([ w/2 - r, -h/2 + r]) circle(r=r);
        translate([-w/2 + r,  h/2 - r]) circle(r=r);
        translate([ w/2 - r,  h/2 - r]) circle(r=r);
    }
}

module rounded_box(w, h, d, r) {
    linear_extrude(height=d) {
        rounded_rect(w, h, r);
    }
}

module keyhole_slot() {
    union() {
        translate([0, -6, -0.5])
            cylinder(d=8.5, h=wall + 2);
        translate([-4.5/2, -6, -0.5])
            cube([4.5, 12, wall + 2]);
        translate([0, 6, -0.5])
            cylinder(d=4.5, h=wall + 2);
        translate([0, -6, wall - 1.2])
            cylinder(d1=8.5, d2=11.5, h=2);
        translate([-11.5/2, -6, wall - 1.2])
            cube([11.5, 12, 2]);
        translate([0, 6, wall - 1.2])
            cylinder(d=11.5, h=2);
    }
}

// 18650 Snap-in Battery Cradle with Central Snap Clamp
module battery_18650_holder() {
    cradle_wall = 2.0;
    snap_w = 24.0;
    overlap = 0.3; // Overlap with floor ensures clean 2-manifold union

    translate([0, bat_y_pos, wall - overlap]) {
        // 1. Central Snap Clamp with flexible C-arms
        difference() {
            translate([-snap_w/2, -(bat_r + cradle_wall), 0])
                cube([snap_w, (bat_r + cradle_wall)*2, bat_r + 4.5 + overlap]);

            // Cylindrical battery bed
            translate([-snap_w/2 - 1, 0, bat_r + overlap])
                rotate([0, 90, 0])
                    cylinder(r=bat_r, h=snap_w + 2, $fn=64);

            // Top snap entry throat (16.6mm wide, flexes open over 18.6mm cell)
            translate([-snap_w/2 - 1, -8.3, bat_r + overlap + 1.8])
                cube([snap_w + 2, 16.6, 12]);

            // 45-degree lead-in ramps on top lip for smooth press-fit insertion
            translate([-snap_w/2 - 1, -8.3, bat_r + overlap + 4.5])
                rotate([45, 0, 0])
                    cube([snap_w + 2, 6, 6]);
            translate([-snap_w/2 - 1, 8.3, bat_r + overlap + 4.5])
                rotate([-45, 0, 0])
                    cube([snap_w + 2, 6, 6]);
        }

        // 2. End-stop walls with wire pass-through notches
        for (sx = [-1, 1]) {
            translate([sx * (bat_len/2 + 1.0), 0, 0]) {
                difference() {
                    translate([-1.0, -(bat_r + 1.5), 0])
                        cube([2.0, (bat_r + 1.5)*2, bat_r + 2.0 + overlap]);
                    // Wire pass-through notch at center for battery leads
                    translate([-2.0, -3.0, overlap])
                        cube([4.0, 6.0, bat_r + 3.0]);
                }
            }
        }

        // 3. Side cradle support saddles near both ends
        for (sx = [-22, 22]) {
            translate([sx - 4, 0, 0]) {
                difference() {
                    translate([0, -(bat_r + cradle_wall), 0])
                        cube([8, (bat_r + cradle_wall)*2, bat_r + overlap]);
                    translate([-1, 0, bat_r + overlap])
                        rotate([0, 90, 0])
                            cylinder(r=bat_r, h=10, $fn=64);
                }
            }
        }
    }
}

// Front bezel internal mounting bosses for Mellemstykke (well away from the 170x100 cutout)
mid_mount_x = 98.0; // Spacing: X = +/- 98mm (in the wide 228mm enclosure)
mid_mount_y = 54.0; // Spacing: Y = +/- 54mm

// =============================================================================
// 1. FRONT BEZEL (FLUSH SUNKEN DISPLAY + SOLID SHELF + 170x100 CUTOUT + BOSSES)
// =============================================================================
module front_bezel() {
    pw = glass_w + 2 * glass_clearance; // 193.86mm
    ph = glass_h + 2 * glass_clearance; // 111.66mm
    pr = glass_r + glass_clearance;

    // Shelf geometry:
    // Front face is at Z = front_depth (8.0mm)
    // Glass pocket drops by 1.8mm -> shelf floor is at Z = 6.2mm
    // Shelf is 2.5mm thick -> internal cavity stops at Z = 3.7mm
    // Bosses extend from Z = 3.7mm down to Z = 0 (height = 3.7mm)
    shelf_floor_z   = front_depth - glass_pocket_d; // 6.2mm
    shelf_thickness = 2.5;                           // 2.5mm thick solid continuous ledge!
    cavity_ceil_z   = shelf_floor_z - shelf_thickness; // 3.7mm

    union() {
        difference() {
            rounded_box(outer_w, outer_h, front_depth, corner_r);

            // 1. Flush Sunken Glass Pocket (1.8mm deep from front face, down to Z = 6.2mm)
            translate([0, 0, shelf_floor_z])
                linear_extrude(height=glass_pocket_d + 1)
                    rounded_rect(pw, ph, pr);

            // 2. LCD Metal Body Pass-Through Cutout: EXACTLY 170mm x 100mm (Through the shelf)
            translate([0, 0, -1])
                linear_extrude(height=front_depth + 2)
                    rounded_rect(170.0, 100.0, 4.0);

            // 3. Internal cavity hollow (STOPS at Z = 3.7mm to GUARANTEE 2.5mm solid shelf!)
            translate([0, 0, -0.5])
                linear_extrude(height=cavity_ceil_z + 0.5)
                    rounded_rect(outer_w - 2 * wall, outer_h - 2 * wall, corner_r - wall);

            // 4. Snap-fit female retention grooves (inside top & bottom walls)
            for (sy = [-1, 1]) {
                for (sx = [-50, 50]) {
                    translate([sx, sy * (outer_h / 2 - wall + 0.1), 2.5])
                        rotate([0, 90, 0])
                            cylinder(r=0.9, h=16, center=true);
                }
            }

            // 5. Pry Slots (2x on bottom and side edges for tool-free disassembly)
            translate([0, -outer_h/2, 0])
                cube([14, wall * 2, 2.0], center=true);
            translate([outer_w/2, 0, 0])
                cube([wall * 2, 14, 2.0], center=true);
        }

        // 6. The 4 Solid Mounting Bosses for Mellemstykke:
        // Grown DIRECTLY from the 2.5mm solid shelf (from Z = 3.7 down to Z = 0)
        // Positioned safely away from the 170x100mm hole
        for (sx = [-1, 1]) {
            for (sy = [-1, 1]) {
                translate([sx * mid_mount_x, sy * mid_mount_y, 0]) {
                    difference() {
                        cylinder(d=10.0, h=cavity_ceil_z, $fn=32);
                        // M3 pilot hole (depth 4.5mm from Z=0, does NOT pierce through front)
                        translate([0, 0, -0.5])
                            cylinder(d=2.8, h=5.0, $fn=24);
                    }
                }
            }
        }
    }
}

// =============================================================================
// 2. MELLEMSTYKKE (DEDICATED DISPLAY CARRIER & MOUNTING MID-PLATE)
// Screws to back of display via 4x Waveshare factory M3 standoffs
// Screws to Front Bezel on inside via 4x outer corner bosses (X=+/-98, Y=+/-54)
// =============================================================================
module mellemstykke() {
    mid_t = 2.5; // Sturdy 2.5mm carrier plate
    
    difference() {
        union() {
            // Main carrier plate (214 x 118 mm fits inside the 223.2 x 121.2 cavity)
            rounded_box(214, 118, mid_t, 7.0);
            
            // Stiffening outer perimeter rib
            difference() {
                rounded_box(214, 118, mid_t + 1.5, 7.0);
                translate([0, 0, -1])
                    rounded_box(208, 112, mid_t + 3.5, 5.0);
            }
        }
        
        // 1. Center opening for ESP32-S3 PCB, buttons and connectors
        // PCB is 106x73mm centered horizontally around X=-1.53, Y=+0.975
        translate([-1.5, 1.0, -1])
            rounded_box(114, 76, mid_t + 4, 3.0);
            
        // 2. Left-side port clearance cutout (wide clearance for USB-C plugs)
        translate([-82, 0, -1])
            cube([44, 46, mid_t + 4], center=true);
            
        // 3. Bottom battery clearance cutout (leaves extra room for 18650 wires)
        translate([0, -42, -1])
            rounded_box(88, 22, mid_t + 4, 3.0);
            
        // 4. 4x Screw holes for Waveshare display factory M3 standoffs
        for (pt = [
            [ws_standoff_x1, ws_standoff_y1],
            [ws_standoff_x1, ws_standoff_y2],
            [ws_standoff_x2, ws_standoff_y1],
            [ws_standoff_x2, ws_standoff_y2]
        ]) {
            translate([pt[0], pt[1], -1]) {
                cylinder(d=3.4, h=mid_t + 4, $fn=24);
                // Counterbore for M3 screw head on back face
                translate([0, 0, mid_t + 1 - 1.2])
                    cylinder(d=6.2, h=3.0, $fn=32);
            }
        }
        
        // 5. 4x Screw holes for mounting Mellemstykke to Front Bezel bosses
        for (sx = [-1, 1]) {
            for (sy = [-1, 1]) {
                translate([sx * mid_mount_x, sy * mid_mount_y, -1]) {
                    cylinder(d=3.4, h=mid_t + 4, $fn=24);
                    // Counterbore for M3 screw head
                    translate([0, 0, mid_t + 1 - 1.2])
                        cylinder(d=6.2, h=3.0, $fn=32);
                }
            }
        }
    }
}

// =============================================================================
// 3. BACK CASE (WALL MOUNT + 18650 CRADLE + UNIVERSAL PORTS + SNAP-FIT LIP)
// 228mm wide with zero bottom stag
// =============================================================================
module back_case() {
    union() {
        difference() {
            union() {
                rounded_box(outer_w, outer_h, back_depth, corner_r);

                // Snap-fit male mating lip (slides inside front bezel)
                translate([0, 0, back_depth]) {
                    difference() {
                        linear_extrude(height=lip_height)
                            rounded_rect(outer_w - 2*wall - 0.3, outer_h - 2*wall - 0.3, corner_r - wall);
                        translate([0, 0, -1])
                            linear_extrude(height=lip_height + 2)
                                rounded_rect(outer_w - 4*wall, outer_h - 4*wall, corner_r - 2*wall);
                    }
                    for (sy = [-1, 1]) {
                        for (sx = [-50, 50]) {
                            translate([sx, sy * ((outer_h - 2*wall - 0.3)/2), lip_height - 1.4])
                                rotate([0, 90, 0])
                                    cylinder(r=0.75, h=14, center=true);
                        }
                    }
                }
            }

            // 1. Main hollow internal volume
            translate([0, 0, wall])
                linear_extrude(height=back_depth + lip_height + 2)
                    rounded_rect(outer_w - 2 * wall, outer_h - 2 * wall, corner_r - wall);

            // 2. Wall Mounting: 2x Keyhole slots (160mm horizontal spacing)
            translate([-80, 22, 0]) keyhole_slot();
            translate([ 80, 22, 0]) keyhole_slot();

            // 3. Central Rear Cable Pass-through (for in-wall wire box / conduit)
            translate([0, -6, -1])
                linear_extrude(height=wall + 2)
                    rounded_rect(38, 22, 4.0);

            // 4. Bottom Surface Raceway Cable Knockout (for CAN & external power)
            translate([0, -outer_h/2, wall + 2])
                cube([22, wall * 3, 10], center=true);

            // 5. Port Cutouts (2x USB-C + MicroSD card slot) on LEFT side ONLY (-X)
            // Right side (+X) is completely solid and closed!
            translate([-outer_w/2, 12, back_depth - 7])
                cube([wall * 3, 13.0, 8.0], center=true);
            translate([-outer_w/2, -8, back_depth - 7])
                cube([wall * 3, 13.0, 8.0], center=true);
            translate([-outer_w/2, -28, back_depth - 7])
                cube([wall * 3, 15.5, 4.5], center=true);

            // 6. Passive Cooling Ventilation Slots (over ESP32-S3 CPU & DC-DC areas)
            for (i = [-5:5]) {
                translate([i * 6.5, 42, -1])
                    linear_extrude(height=wall + 2)
                        rounded_rect(2.5, 18, 1.25);
            }
        }

        // 18650 Battery Snap Cradle cleanly merged into the floor
        // (Zero bottom stag/pillars - completely clean floor!)
        battery_18650_holder();
    }
}

// =============================================================================
// EXECUTION / RENDER SELECTOR
// =============================================================================
if (part == "front") {
    front_bezel();
} else if (part == "back") {
    back_case();
} else if (part == "mid" || part == "mellemstykke") {
    mellemstykke();
} else {
    // Exploded view
    color("LightSlateGray", 0.9)
        translate([0, 0, back_depth + lip_height + exploded_distance + 15])
            front_bezel();
    color("MediumPurple", 0.95)
        translate([0, 0, back_depth + lip_height + exploded_distance - 2])
            mellemstykke();
    color("DarkSlateGray", 0.95)
        back_case();
}

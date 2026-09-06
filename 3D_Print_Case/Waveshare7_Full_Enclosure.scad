// =============================================================================
// PARAMETRIC WALL-MOUNT ENCLOSURE FOR WAVESHARE ESP32-S3-TOUCH-LCD-7 (REV 1.2)
// Designed for Bambu Lab 3D Printers (Bambu Studio / Bambu P1S / X1C / A1 / H2D)
// -----------------------------------------------------------------------------
// Features:
// 1. Flush Sunken Front Bezel (glass is 100% plant / flush with front face)
// 2. Snap-Fit tool-free closure with 2x pry-slots for easy disassembly
// 3. Wall-mount keyhole slots on back (horizontal spacing 120mm)
// 4. Extra internal depth (25mm internal clearance for CAN, LiPo, DC-DC buck, wires)
// 5. Ports for 2x USB-C (UART/CDC) and MicroSD slot
// 6. Rear and bottom cable feedthroughs + cooling ventilation slots
// 7. 4x M3 mounting standoffs inside front bezel matching Waveshare PCB ears
// =============================================================================

/* [Render Mode] */
part = "both"; // [both, front, back]
exploded_distance = 35;

/* [Display Specifications (Waveshare 7.0" Touch)] */
glass_w        = 192.96;
glass_h        = 110.76;
glass_r        = 7.88;
glass_pocket_d = 2.0;     // Recess depth so touch glass is 100% flush with bezel face
active_w       = 155.0;
active_h       = 87.0;
glass_clearance= 0.45;    // Fit clearance per side (0.9mm total W/H)

mount_hole_x_dist = 126.20;
mount_hole_y_dist = 65.65;
mount_boss_d      = 6.5;
mount_hole_d      = 2.8;

/* [Enclosure Dimensions] */
wall           = 2.4;
outer_w        = 208.0;
outer_h        = 126.0;
corner_r       = 9.5;
total_depth    = 30.0;
front_depth    = 7.5;
back_depth     = total_depth - front_depth; // 22.5mm
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

module front_bezel() {
    pw = glass_w + 2 * glass_clearance;
    ph = glass_h + 2 * glass_clearance;
    pr = glass_r + glass_clearance;

    difference() {
        union() {
            rounded_box(outer_w, outer_h, front_depth, corner_r);
            for (sx = [-1, 1]) {
                for (sy = [-1, 1]) {
                    translate([sx * mount_hole_x_dist / 2, sy * mount_hole_y_dist / 2, 0]) {
                        cylinder(d=mount_boss_d, h=front_depth + 3.0);
                    }
                }
            }
        }

        // 1. Flush Sunken Glass Pocket
        translate([0, 0, front_depth - glass_pocket_d])
            linear_extrude(height=glass_pocket_d + 1)
                rounded_rect(pw, ph, pr);

        // 2. Active Screen Viewing Window
        translate([0, 0, -1])
            linear_extrude(height=front_depth + 2)
                rounded_rect(active_w, active_h, 3.0);

        // 3. Internal cavity hollow
        translate([0, 0, -0.5])
            linear_extrude(height=front_depth - 2.0)
                rounded_rect(outer_w - 2 * wall, outer_h - 2 * wall, corner_r - wall);

        // 4. M3 Screw Holes in standoffs
        for (sx = [-1, 1]) {
            for (sy = [-1, 1]) {
                translate([sx * mount_hole_x_dist / 2, sy * mount_hole_y_dist / 2, front_depth - 7.0]) {
                    cylinder(d=mount_hole_d, h=12);
                }
            }
        }

        // 5. Snap-fit female retention grooves
        for (sy = [-1, 1]) {
            for (sx = [-50, 50]) {
                translate([sx, sy * (outer_h / 2 - wall + 0.1), 3.0])
                    rotate([0, 90, 0])
                        cylinder(r=0.9, h=16, center=true);
            }
        }

        // 6. Pry Slots
        translate([0, -outer_h/2, 0])
            cube([14, wall * 2, 2.0], center=true);
        translate([outer_w/2, 0, 0])
            cube([wall * 2, 14, 2.0], center=true);
    }
}

module back_case() {
    difference() {
        union() {
            rounded_box(outer_w, outer_h, back_depth, corner_r);
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

        // Internal volume
        translate([0, 0, wall])
            linear_extrude(height=back_depth + lip_height + 2)
                rounded_rect(outer_w - 2 * wall, outer_h - 2 * wall, corner_r - wall);

        // Keyholes
        translate([-60, 20, 0]) keyhole_slot();
        translate([ 60, 20, 0]) keyhole_slot();

        // Rear Cable hole
        translate([0, -18, -1])
            linear_extrude(height=wall + 2)
                rounded_rect(38, 24, 4.0);

        // Bottom Cable Knockout
        translate([0, -outer_h/2, wall + 2])
            cube([20, wall * 3, 10], center=true);

        // Right USB-C Ports
        translate([outer_w/2, 10, back_depth - 7])
            cube([wall * 3, 12.5, 7.5], center=true);
        translate([outer_w/2, -10, back_depth - 7])
            cube([wall * 3, 12.5, 7.5], center=true);
        translate([outer_w/2, -30, back_depth - 7])
            cube([wall * 3, 15.0, 4.0], center=true);

        // Cooling Vents
        for (i = [-4:4]) {
            translate([i * 6.5, 38, -1])
                linear_extrude(height=wall + 2)
                    rounded_rect(2.5, 20, 1.25);
        }
    }
}

if (part == "front") {
    translate([0, 0, front_depth])
        rotate([180, 0, 0])
            front_bezel();
} else if (part == "back") {
    back_case();
} else {
    color("LightSlateGray", 0.9)
        translate([0, 0, back_depth + lip_height + exploded_distance])
            front_bezel();
    color("DarkSlateGray", 0.95)
        back_case();
}

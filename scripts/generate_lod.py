# GENERATED WITH AI
# Blender Python script to generate 5 LOD levels from horse_statue_01_4k.obj
# Preserves UVs, normals, and (if present) materials/mtllib.
#
# Run with:
#   cd /home/dtaunt/dev/iron-cube && blender -b -P generate_lod.py 2>&1
#
# LOD ratios (fraction of original faces kept):
#   LOD 0: 1.000  (full detail)
#   LOD 1: 0.500  (half)
#   LOD 2: 0.250  (quarter)
#   LOD 3: 0.125  (eighth)
#   LOD 4: 0.0625 (sixteenth)

import bpy
import os
import sys
import shutil

# --- Configuration ---
src_obj = "/home/dtaunt/dev/iron-cube/assets/asteroid/asteroid.obj"
out_dir = "/home/dtaunt/dev/iron-cube/assets/asteroid_lod"
base_name = "asteroid"
num_lods = 10

lod_ratios = [0.5 ** i for i in range(num_lods)]
lod_offset = 5  # output as LOD_5..LOD_10
min_faces = 4  # safety floor so decimate never collapses a mesh to nothing


# ---------------------------------------------------------------------------
# OBJ parsing
# ---------------------------------------------------------------------------
def parse_obj(filepath):
    """Parse an OBJ file, keeping positions, UVs, normals, per-corner
    v/vt/vn indices, and (best-effort) material assignment per face.

    Returns:
        positions: list of (x, y, z)
        uvs:       list of (u, v)
        normals:   list of (x, y, z)
        faces:     list of triangles; each triangle is a list of 3 corners;
                   each corner is (pos_idx, uv_idx or None, norm_idx or None)
        face_mtl:  list, one material name (or None) per output triangle,
                   aligned with `faces`
        mtllib:    the mtllib filename referenced in the source file, or None
    """
    positions, uvs, normals = [], [], []
    faces, face_mtl = [], []
    current_mtl = None
    mtllib = None

    def resolve(idx, count):
        # OBJ indices are 1-based; negative indices are relative to the
        # current running count.
        if idx > 0:
            return idx - 1
        return count + idx

    with open(filepath) as f:
        for line in f:
            parts = line.split()
            if not parts:
                continue
            tag = parts[0]

            if tag == "v" and len(parts) >= 4:
                positions.append((float(parts[1]), float(parts[2]), float(parts[3])))
            elif tag == "vt" and len(parts) >= 3:
                uvs.append((float(parts[1]), float(parts[2])))
            elif tag == "vn" and len(parts) >= 4:
                normals.append((float(parts[1]), float(parts[2]), float(parts[3])))
            elif tag == "mtllib" and len(parts) >= 2:
                mtllib = parts[1]
            elif tag == "usemtl" and len(parts) >= 2:
                current_mtl = parts[1]
            elif tag == "f" and len(parts) >= 4:
                corners = []
                for vert_ref in parts[1:]:
                    comps = vert_ref.split("/")
                    p_idx = resolve(int(comps[0]), len(positions))
                    t_idx = None
                    n_idx = None
                    if len(comps) >= 2 and comps[1] != "":
                        t_idx = resolve(int(comps[1]), len(uvs))
                    if len(comps) >= 3 and comps[2] != "":
                        n_idx = resolve(int(comps[2]), len(normals))
                    corners.append((p_idx, t_idx, n_idx))
                # Fan-triangulate any n-gon
                for i in range(1, len(corners) - 1):
                    faces.append([corners[0], corners[i], corners[i + 1]])
                    face_mtl.append(current_mtl)

    return positions, uvs, normals, faces, face_mtl, mtllib


# ---------------------------------------------------------------------------
# OBJ writing
# ---------------------------------------------------------------------------
def write_obj(filepath, positions, uvs, normals, faces, face_mtl, mtllib_name):
    """Write OBJ with v/vt/vn and per-corner v/vt/vn face indices.
    UV/normal lists passed in are already deduplicated per-file."""
    has_uv = any(c[1] is not None for tri in faces for c in tri)
    has_n = any(c[2] is not None for tri in faces for c in tri)

    # Group faces by material to minimize usemtl toggles, preserving
    # relative order within each group.
    order = sorted(range(len(faces)), key=lambda i: (face_mtl[i] or "", i))

    with open(filepath, "w") as f:
        f.write(f"# {len(faces)} faces\n")
        if mtllib_name:
            f.write(f"mtllib {mtllib_name}\n")
        for v in positions:
            f.write(f"v {v[0]:.6f} {v[1]:.6f} {v[2]:.6f}\n")
        if has_uv:
            for uv in uvs:
                f.write(f"vt {uv[0]:.6f} {uv[1]:.6f}\n")
        if has_n:
            for n in normals:
                f.write(f"vn {n[0]:.6f} {n[1]:.6f} {n[2]:.6f}\n")

        last_mtl = "__unset__"
        for i in order:
            mtl = face_mtl[i]
            if mtllib_name and mtl != last_mtl:
                f.write(f"usemtl {mtl if mtl else 'default'}\n")
                last_mtl = mtl
            tri = faces[i]
            tokens = []
            for p, t, n in tri:
                tok = f"{p + 1}"
                if has_uv:
                    tok += f"/{(t + 1) if t is not None else ''}"
                elif has_n:
                    tok += "/"
                if has_n:
                    tok += f"/{(n + 1) if n is not None else ''}"
                tokens.append(tok)
            f.write("f " + " ".join(tokens) + "\n")


# ---------------------------------------------------------------------------
# Blender mesh construction
# ---------------------------------------------------------------------------
def create_blender_mesh(name, positions, uvs, normals, faces, face_mtl, material_names):
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(positions, [], [[c[0] for c in tri] for tri in faces])
    mesh.update()

    has_uv = any(c[1] is not None for tri in faces for c in tri)
    has_n = any(c[2] is not None for tri in faces for c in tri)

    if has_uv:
        uv_layer = mesh.uv_layers.new(name="UVMap")
        for poly in mesh.polygons:
            tri = faces[poly.index]
            for local_i, loop_i in enumerate(poly.loop_indices):
                t_idx = tri[local_i][1]
                uv_layer.data[loop_i].uv = (
                    uvs[t_idx] if t_idx is not None else (0.0, 0.0)
                )

    if has_n:
        loop_normals = [(0.0, 0.0, 1.0)] * len(mesh.loops)
        for poly in mesh.polygons:
            tri = faces[poly.index]
            for local_i, loop_i in enumerate(poly.loop_indices):
                n_idx = tri[local_i][2]
                if n_idx is not None:
                    loop_normals[loop_i] = normals[n_idx]
        try:
            mesh.normals_split_custom_set(loop_normals)
        except AttributeError:
            # Removed in newer Blender versions; shading will fall back
            # to auto-generated normals instead of the source file's.
            print(
                "  (custom split normals API unavailable on this Blender "
                "version; using auto-generated normals instead)"
            )

    if material_names:
        for m_name in material_names:
            mat = bpy.data.materials.get(m_name) or bpy.data.materials.new(m_name)
            mesh.materials.append(mat)
        name_to_slot = {m_name: i for i, m_name in enumerate(material_names)}
        for poly in mesh.polygons:
            mtl = face_mtl[poly.index]
            poly.material_index = name_to_slot.get(mtl, 0)

    mesh.validate()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    return obj


def read_loop_normals(mesh):
    """Return a list of per-loop normal vectors, handling API differences
    across Blender versions."""
    try:
        mesh.calc_normals_split()
        return [tuple(loop.normal) for loop in mesh.loops]
    except AttributeError:
        pass
    if hasattr(mesh, "corner_normals"):
        try:
            return [tuple(cn.vector) for cn in mesh.corner_normals]
        except Exception:
            pass
    # Last resort: per-vertex normals repeated per loop
    return [tuple(mesh.vertices[loop.vertex_index].normal) for loop in mesh.loops]


def decimate_and_read(source_obj, ratio, target_name, material_names):
    """Duplicate source mesh, apply Decimate + Triangulate via the
    dependency graph (no bpy.ops -> reliable in background mode), and
    return (positions, uvs, normals, faces, face_mtl) for the result,
    with uvs/normals deduplicated."""
    lod_mesh = source_obj.data.copy()
    lod_obj = bpy.data.objects.new(target_name, lod_mesh)
    bpy.context.collection.objects.link(lod_obj)

    dec = lod_obj.modifiers.new(name="Decimate", type="DECIMATE")
    dec.decimate_type = "COLLAPSE"
    dec.ratio = ratio
    dec.use_dissolve_boundaries = True
    # Symmetry is off by default; only touch these if the Blender version
    # in use actually exposes them (the attribute names/shape have changed
    # across Blender releases).
    if hasattr(dec, "use_symmetry"):
        dec.use_symmetry = False
    for axis_attr in (
        "use_symmetry_axis_x",
        "use_symmetry_axis_y",
        "use_symmetry_axis_z",
    ):
        if hasattr(dec, axis_attr):
            setattr(dec, axis_attr, False)
    if hasattr(dec, "use_collapse_triangulate"):
        dec.use_collapse_triangulate = True

    tri = lod_obj.modifiers.new(name="Triangulate", type="TRIANGULATE")
    tri.quad_method = "BEAUTY"
    tri.ngon_method = "BEAUTY"

    depsgraph = bpy.context.evaluated_depsgraph_get()
    eval_obj = lod_obj.evaluated_get(depsgraph)
    eval_mesh = eval_obj.to_mesh()

    positions = [(v.co.x, v.co.y, v.co.z) for v in eval_mesh.vertices]

    has_uv = bool(eval_mesh.uv_layers.active)
    uv_layer = eval_mesh.uv_layers.active.data if has_uv else None
    loop_normals = read_loop_normals(eval_mesh)

    uv_map, uv_list = {}, []
    n_map, n_list = {}, []

    def dedup_uv(uv):
        key = (round(uv[0], 6), round(uv[1], 6))
        if key not in uv_map:
            uv_map[key] = len(uv_list)
            uv_list.append(key)
        return uv_map[key]

    def dedup_normal(n):
        key = (round(n[0], 5), round(n[1], 5), round(n[2], 5))
        if key not in n_map:
            n_map[key] = len(n_list)
            n_list.append(key)
        return n_map[key]

    faces, face_mtl = [], []
    slot_names = (
        [mesh_slot.name for mesh_slot in eval_mesh.materials]
        if eval_mesh.materials
        else []
    )

    for poly in eval_mesh.polygons:
        tri = []
        for loop_i in poly.loop_indices:
            loop = eval_mesh.loops[loop_i]
            p_idx = loop.vertex_index
            t_idx = dedup_uv(uv_layer[loop_i].uv) if has_uv else None
            n_idx = dedup_normal(loop_normals[loop_i]) if loop_normals else None
            tri.append((p_idx, t_idx, n_idx))
        faces.append(tri)
        mtl_name = (
            slot_names[poly.material_index]
            if slot_names and poly.material_index < len(slot_names)
            else None
        )
        face_mtl.append(mtl_name)

    eval_obj.to_mesh_clear()
    bpy.data.objects.remove(lod_obj, do_unlink=True)
    bpy.data.meshes.remove(lod_mesh)

    return positions, uv_list, n_list, faces, face_mtl


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
if not os.path.isfile(src_obj):
    print(f"ERROR: source OBJ not found: {src_obj}")
    sys.exit(1)

os.makedirs(out_dir, exist_ok=True)

print(f"Parsing {src_obj} ...")
positions, uvs, normals, faces, face_mtl, mtllib = parse_obj(src_obj)
print(
    f"  {len(positions)} vertices, {len(faces)} faces, "
    f"{len(uvs)} uvs, {len(normals)} normals, mtllib={mtllib}"
)

if not positions or not faces:
    print("ERROR: parsed 0 vertices/faces, aborting.")
    sys.exit(1)

material_names = sorted({m for m in face_mtl if m is not None})

# Copy the referenced .mtl next to the source obj, if any, into out_dir.
out_mtl_name = None
if mtllib:
    src_mtl_path = os.path.join(os.path.dirname(src_obj), mtllib)
    if os.path.isfile(src_mtl_path):
        out_mtl_name = f"{base_name}.mtl"
        shutil.copyfile(src_mtl_path, os.path.join(out_dir, out_mtl_name))
        print(f"  Copied material file -> {out_mtl_name}")
    else:
        print(
            f"  WARNING: mtllib '{mtllib}' referenced but not found next to source obj; "
            f"skipping material file copy."
        )
        mtllib = None

source_obj = create_blender_mesh(
    "SourceMesh", positions, uvs, normals, faces, face_mtl, material_names
)

for lod_index, ratio in enumerate(lod_ratios):
    lod_num = lod_index + lod_offset
    print(f"\nGenerating LOD {lod_num} (ratio={ratio})...")

    if ratio >= 1.0:
        lod_positions, lod_uvs, lod_normals = positions, uvs, normals
        lod_faces, lod_face_mtl = faces, face_mtl
    else:
        effective_ratio = max(ratio, min_faces / max(len(faces), 1))
        lod_positions, lod_uvs, lod_normals, lod_faces, lod_face_mtl = (
            decimate_and_read(
                source_obj, effective_ratio, f"LOD_{lod_num}", material_names
            )
        )

    print(
        f"  {len(lod_positions)} vertices, {len(lod_faces)} faces, "
        f"{len(lod_uvs)} uvs, {len(lod_normals)} normals"
    )

    out_path = os.path.join(out_dir, f"{base_name}_LOD_{lod_num}.obj")
    write_obj(
        out_path,
        lod_positions,
        lod_uvs,
        lod_normals,
        lod_faces,
        lod_face_mtl,
        out_mtl_name,
    )
    print(f"  Written to {out_path}")

bpy.data.objects.remove(source_obj, do_unlink=True)

print(f"\nDone. {len(lod_ratios)} LOD files in {out_dir}/")

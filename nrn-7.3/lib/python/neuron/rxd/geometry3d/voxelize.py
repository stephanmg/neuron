import ctng
import scalarField
import numpy

def voxelize(source, dx=0.25, xlo=None, xhi=None, ylo=None, yhi=None, zlo=None, zhi=None, n_soma_step=100):
    """
    Generates a cartesian mesh of the volume of a neuron.
    
    Parameters
    ----------    
    source : :func:`list`, ``nrn.SectionList``, or ``nrn.Import3D``
        The geometry to mesh.
    dx : double, optional
        Mesh step size.
    xlo : double, optional
        Minimum x value. If omitted or None, uses minimum x value in the geometry.
    xhi : double, optional
        Maximum x value. If omitted or None, uses maximum x value in the geometry.
    ylo : double, optional
        Minimum y value. If omitted or None, uses minimum y value in the geometry.
    yhi : double, optional
        Maximum y value. If omitted or None, uses maximum y value in the geometry.
    zlo : double, optional
        Minimum z value. If omitted or None, uses minimum z value in the geometry.
    zhi : double, optional
        Maximum z value. If omitted or None, uses maximum z value in the geometry.
    n_soma_step : integer, optional
        Number of pieces to slice a soma outline into.
        

    Returns
    -------
    result : :class:`ScalarField`
        The mesh. Values are scalars, but may be used as True inside the
        geometry and False outside.


    Examples
    --------
    
    Basic usage:

    >>> mesh = geometry3d.voxelize(h.allsec())
    
    Full example, using :mod:`pyplot`:

    >>> s1, s2, s3 = [h.Section() for i in xrange(3)]
    >>> for sec in [s2, s3]: ignore_return = sec.connect(s1)
    ... 
    >>> for sec in h.allsec():
    ...     sec.diam = 1
    ...     sec.L = 5
    ... 
    >>> mesh = geometry3d.voxelize(h.allsec(), dx=.1)
    >>> for i in xrange(10):
    ...     ignore_return = pyplot.subplot(2, 5, i + 1)
    ...     ignore_return = pyplot.imshow(mesh.values[:, :, i])
    ...     ignore_return = pyplot.xticks([])
    ...     ignore_return = pyplot.yticks([])
    ... 
    >>> pyplot.show()
    
    .. plot::
     
        from neuron import h
        from matplotlib import pyplot
        from neuron.rxd import geometry3d

        s1, s2, s3 = [h.Section() for i in xrange(3)]
        for sec in [s2, s3]: ignore_return = sec.connect(s1)

        for sec in h.allsec():
            sec.diam = 1
            sec.L = 5

        mesh = geometry3d.voxelize(h.allsec(), dx=.1)
        for i in xrange(10):
            ignore_return = pyplot.subplot(2, 5, i + 1)
            ignore_return = pyplot.imshow(mesh.values[:, :, i])
            ignore_return = pyplot.xticks([])
            ignore_return = pyplot.yticks([])

        pyplot.show()

    
    
    .. note::
        The use of Import3D objects is recommended over lists of sections
        because the former preserves the soma outline information while
        the later does not. Up to one soma outline is currently supported.
    """
    
    objects = ctng.constructive_neuronal_geometry(source, n_soma_step, dx)
    
    if xlo is None: xlo = min(obj.xlo for obj in objects)
    if ylo is None: ylo = min(obj.ylo for obj in objects)
    if zlo is None: zlo = min(obj.zlo for obj in objects)
    if xhi is None: xhi = max(obj.xhi for obj in objects)
    if yhi is None: yhi = max(obj.yhi for obj in objects)
    if zhi is None: zhi = max(obj.zhi for obj in objects)
    
    mesh = scalarField.ScalarField(xlo, xhi, ylo, yhi, zlo, zhi, dx, dtype='B')
    grid = mesh.values
       
    # figure out which objects go with which x/y/z values
    x_objs = {x: [obj for obj in objects if obj.xlo < x < obj.xhi] for x in mesh.xs}
    y_objs = {y: [obj for obj in objects if obj.ylo < y < obj.yhi] for y in mesh.ys}
    z_objs = {z: [obj for obj in objects if obj.zlo < z < obj.zhi] for z in mesh.zs}

    for i, x in enumerate(mesh.xs):
        x_obj = set(x_objs[x])
        for j, y in enumerate(mesh.ys):
            xy_obj = x_obj.intersection(y_objs[y])
            for k, z in enumerate(mesh.zs):
                grid[i, j, k] = is_inside(x, y, z, xy_obj.intersection(z_objs[z]))
    
    return mesh

            
            
# inside the neuron if inside of any of its parts
def is_inside(x, y, z, active_objs):
    return 1 if any(obj.distance(x, y, z) <= 0 for obj in active_objs) else 0


/*

Copyright (C) 2012 Michael Goffioul

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if !defined (octave_classdef_h)
#define octave_classdef_h 1

#include <set>
#include <string>

#include "oct-map.h"
#include "oct-refcount.h"
#include "ov-base.h"

class cdef_object;
class cdef_class;
class cdef_property;
class cdef_method;
class cdef_package;

class tree_classdef;

// This is mainly a boostrap class to declare the expected interface.
// The actual base class is cdef_class_base, which is declared after
// cdef_object, such that it can contain cdef_object objects.
class
cdef_object_rep
{
public:
  friend class cdef_object;

public:
  cdef_object_rep (void) : refcount (1) { }

  virtual ~cdef_object_rep (void) { }

  virtual cdef_class get_class (void) const;

  virtual void set_class (const cdef_class&)
    { gripe_invalid_object ("set_class"); }

  virtual cdef_object_rep* clone (void) const
    {
      gripe_invalid_object ("clone");
      return new cdef_object_rep ();
    }

  virtual bool is_class (void) const { return false; }

  virtual bool is_property (void) const { return false; }

  virtual bool is_method (void) const { return false; }

  virtual bool is_package (void) const { return false; }

  virtual void put (const std::string&, const octave_value&)
    { gripe_invalid_object ("put"); }

  virtual octave_value get (const std::string&) const
    {
      gripe_invalid_object ("get");
      return octave_value ();
    }

  virtual octave_value_list
  subsref (const std::string&, const std::list<octave_value_list>&,
           int, size_t&, const cdef_class&)
    {
      gripe_invalid_object ("subsref");
      return octave_value_list ();
    }

  virtual octave_value
  subsasgn (const std::string&, const std::list<octave_value_list>&,
            const octave_value&)
    {
      gripe_invalid_object ("subsasgn");
      return octave_value ();
    }

  virtual string_vector map_keys(void) const;

  virtual bool is_valid (void) const { return false; }

  std::string class_name (void) const;

  virtual void mark_for_construction (const cdef_class&)
    { gripe_invalid_object ("mark_for_construction"); }

  virtual bool is_constructed_for (const cdef_class&) const
    {
      gripe_invalid_object ("is_constructed_for");
      return false;
    }

  virtual bool is_partially_constructed_for (const cdef_class&) const
    {
      gripe_invalid_object ("is_partially_constructed_for");
      return false;
    }

  virtual void mark_as_constructed (void)
    { gripe_invalid_object ("mark_as_constructed"); }

  virtual void mark_as_constructed (const cdef_class&)
    { gripe_invalid_object ("mark_as_constructed"); }

  virtual bool is_constructed (void) const
    {
      gripe_invalid_object ("is_constructed");
      return false;
    }

protected:
  /* reference count */
  octave_refcount<int> refcount;

protected:
  /* Restricted copying */
  cdef_object_rep (const cdef_object_rep&)
    : refcount (1) { }

private:
  /* No assignment */
  cdef_object_rep& operator = (const cdef_object_rep& );

  void gripe_invalid_object (const char *who) const
    { error ("%s: invalid object", who); }
};

class
cdef_object
{
public:
  /* FIXME: use a null object */
  cdef_object (void)
      : rep (new cdef_object_rep ()) { }

  cdef_object (const cdef_object& obj)
    : rep (obj.rep)
    {
      rep->refcount++;
    }

  cdef_object (cdef_object_rep *r)
      : rep (r) { }

  virtual ~cdef_object (void)
    {
      if (--rep->refcount == 0)
	delete rep;
    }

  cdef_object& operator = (const cdef_object& obj)
    {
      if (rep != obj.rep)
	{
	  if (--rep->refcount == 0)
	    delete rep;

	  rep = obj.rep;
	  rep->refcount++;
	}

      return *this;
    }

  cdef_class get_class (void) const;

  void set_class (const cdef_class& cls) { rep->set_class (cls); }

  std::string class_name (void) const
    { return rep->class_name (); }

  cdef_object clone (void) const
    { return cdef_object (rep->clone ()); }

  bool is_class (void) const { return rep->is_class (); }

  bool is_property (void) const { return rep->is_property (); }

  bool is_method (void) const { return rep->is_method (); }

  bool is_package (void) const { return rep->is_package (); }

  void put (const std::string& pname, const octave_value& val)
    { rep->put (pname, val); }

  octave_value get (const std::string& pname) const
    { return rep->get (pname); }

  octave_value_list
  subsref (const std::string& type, const std::list<octave_value_list>& idx,
           int nargout, size_t& skip, const cdef_class& context)
    { return rep->subsref (type, idx, nargout, skip, context); }

  octave_value
  subsasgn (const std::string& type, const std::list<octave_value_list>& idx,
            const octave_value& rhs)
    { return rep->subsasgn (type, idx, rhs); }

  string_vector map_keys (void) const { return rep->map_keys (); }

  const cdef_object_rep* get_rep (void) const { return rep; }

  bool ok (void) const { return rep->is_valid (); }

  void mark_for_construction (const cdef_class& cls)
    { rep->mark_for_construction (cls); }

  bool is_constructed (void) const { return rep->is_constructed (); }

  bool is_constructed_for (const cdef_class& cls) const
    { return rep->is_constructed_for (cls); }

  bool is_partially_constructed_for (const cdef_class& cls) const
    { return rep->is_partially_constructed_for (cls); }

  void mark_as_constructed (void) { rep->mark_as_constructed (); }

  void mark_as_constructed (const cdef_class& cls)
    { rep->mark_as_constructed (cls); }

protected:
  cdef_object_rep* get_rep (void) { return rep; }

private:
  cdef_object_rep *rep;
};

class
cdef_object_base : public cdef_object_rep
{
public:
  cdef_object_base (void)
    : cdef_object_rep (), klass ()
    {
      register_object ();
    }

  ~cdef_object_base (void) { unregister_object (); }

  cdef_class get_class (void) const;

  void set_class (const cdef_class& cls);

protected:
  // Restricted copying!
  cdef_object_base (const cdef_object_base& obj)
    : cdef_object_rep (obj), klass (obj.klass)
    {
      register_object ();
    }

private:
  void register_object (void);

  void unregister_object (void);

private:
  // The class of the object
  cdef_object klass;

private:
  // No assignment!
  cdef_object_base& operator = (const cdef_object_base&);
};

class
cdef_object_scalar : public cdef_object_base
{
public:
  cdef_object_scalar (void) : cdef_object_base () { }

  ~cdef_object_scalar (void) { }

  void put (const std::string& pname, const octave_value& val)
    { map.assign (pname, val); }

  octave_value get (const std::string& pname) const
    {
      Cell val = map.contents (pname);

      if (val.numel () > 0)
	return val(0, 0);
      else
	{
	  error ("get: unknown slot: %s", pname.c_str ());
	  return octave_value ();
	}
    }

  octave_value_list
  subsref (const std::string& type, const std::list<octave_value_list>& idx,
           int nargout, size_t& skip, const cdef_class& context);

  octave_value
  subsasgn (const std::string& type, const std::list<octave_value_list>& idx,
            const octave_value& rhs);

  void mark_for_construction (const cdef_class&);

  bool is_constructed_for (const cdef_class& cls) const;

  bool is_partially_constructed_for (const cdef_class& cls) const;

  void mark_as_constructed (void) { ctor_list.clear (); }

  void mark_as_constructed (const cdef_class& cls) { ctor_list.erase (cls); }

  bool is_constructed (void) const { return ctor_list.empty (); }

protected:
  // Object property values
  Octave_map map;

  // Internal/temporary structure used during object construction
  std::map< cdef_class, std::list<cdef_class> > ctor_list;

protected:
  // Restricted object copying!
  cdef_object_scalar (const cdef_object_scalar& obj)
    : cdef_object_base (obj), map (obj.map), ctor_list (obj.ctor_list) { }

private:
  // No assignment!
  cdef_object_scalar& operator = (const cdef_object_scalar&);
};

class
handle_cdef_object : public cdef_object_scalar
{
public:
  handle_cdef_object (void)
      : cdef_object_scalar () { }

  ~handle_cdef_object (void);

  cdef_object_rep* clone (void) const
    {
      handle_cdef_object *obj = const_cast<handle_cdef_object *> (this);
      obj->refcount++;
      return obj;
    }

  bool is_valid (void) const { return true; }

private:
  // No copying
  handle_cdef_object (const handle_cdef_object&);
  handle_cdef_object& operator = (const handle_cdef_object&);
};

class
value_cdef_object : public cdef_object_scalar
{
public:
  value_cdef_object (void)
      : cdef_object_scalar () { }

  ~value_cdef_object (void);

  cdef_object_rep* clone (void) const
    {
      value_cdef_object* obj = new value_cdef_object (*this);
      return obj;
    }

  bool is_valid (void) const { return true; }

private:
  // Private copying!
  value_cdef_object (const value_cdef_object& obj)
    : cdef_object_scalar (obj) { }

  // No assignment!
  value_cdef_object& operator = (const value_cdef_object&);
};

class
cdef_class : public cdef_object
{
private:

  class
  cdef_class_rep : public handle_cdef_object
  {
  public:
    cdef_class_rep (void)
	: handle_cdef_object (), handle_class (false), object_count (0) { }

    cdef_class_rep (const std::list<cdef_class>& superclasses);

    bool is_class (void) const { return true; }

    std::string get_name (void) const
      { return get ("Name").string_value (); }

    void set_name (const std::string& nm) { put ("Name", nm); }

    cdef_method find_method (const std::string& nm, bool local = false);

    void install_method (const cdef_method& meth);

    Cell get_methods (void);

    cdef_property find_property (const std::string& nm);

    void install_property (const cdef_property& prop);

    Cell get_properties (void);

    string_vector get_names (void);

    void set_directory (const std::string& dir) { directory = dir; }

    std::string get_directory (void) const { return directory; }

    void delete_object (cdef_object obj);

    octave_value_list
    subsref_meta (const std::string& type,
                  const std::list<octave_value_list>& idx, int nargout);

    octave_value construct (const octave_value_list& args);

    void initialize_object (cdef_object& obj);

    void run_constructor (cdef_object& obj, const octave_value_list& args);

    void mark_as_handle_class (void) { handle_class = true; }

    bool is_handle_class (void) const { return handle_class; }

    void register_object (void) { object_count++; }

    void unregister_object (void) { object_count--; }

  private:
    void load_all_methods (void);

    void find_names (std::set<std::string>& names, bool all);
    
    void find_properties (std::map<std::string,cdef_property>& props,
                          bool only_inherited);

    void find_methods (std::map<std::string, cdef_method>& meths,
                       bool only_inherited);

    cdef_class wrap (void)
      {
        refcount++;
        return cdef_class (this);
      }

  private:
    // The @-directory were this class is loaded from.
    // (not used yet)
    std::string directory;

    // The methods defined by this class.
    std::map<std::string,cdef_method> method_map;

    // The properties defined by this class.
    std::map<std::string,cdef_property> property_map;

    // TRUE if this class is a handle class. A class is a handle
    // class when the abstract "handle" class is one of its superclasses.
    bool handle_class;

    // The list of super-class constructors that are called implicitly by the
    // the classdef engine when creating an object. These constructors are not
    // called explicitly by the class constructor.
    std::list<cdef_class> implicit_ctor_list;

    // The number of objects of this class.
    octave_refcount<octave_idx_type> object_count;

    // Utility iterator typedef's.
    typedef std::map<std::string,cdef_method>::iterator method_iterator;
    typedef std::map<std::string,cdef_method>::const_iterator method_const_iterator;
    typedef std::map<std::string,cdef_property>::iterator property_iterator;
    typedef std::map<std::string,cdef_property>::const_iterator property_const_iterator;
  };

public:
  // Create and invalid class object
  cdef_class (void)
      : cdef_object () { }

  cdef_class (const std::string& nm,
              const std::list<cdef_class>& superclasses)
      : cdef_object (new cdef_class_rep (superclasses))
    { get_rep ()->set_name (nm); }

  cdef_class (const cdef_class& cls)
      : cdef_object (cls) { }

  cdef_class (const cdef_object& obj)
      : cdef_object (obj)
    {
      // This should never happen...
      if (! is_class ())
	error ("internal error: invalid assignment from %s to meta.class object",
	       class_name ().c_str ());
    }

  cdef_class& operator = (const cdef_class& cls)
    {
      cdef_object::operator= (cls);

      return *this;
    }

  cdef_class& operator = (const cdef_object& obj)
    {
      if (obj.is_class ())
	cdef_object::operator= (obj);
      else
	error ("internal error: invalid assignment from %s to meta.class object",
	       class_name ().c_str ());

      return *this;
    }

  cdef_method find_method (const std::string& nm, bool local = false);

  void install_method (const cdef_method& meth)
    { get_rep ()->install_method (meth); }

  Cell get_methods (void) { return get_rep ()->get_methods (); }

  cdef_property find_property (const std::string& nm);
  
  void install_property (const cdef_property& prop)
    { get_rep ()->install_property (prop); }

  Cell get_properties (void) { return get_rep ()->get_properties (); }

  string_vector get_names (void) { return get_rep ()->get_names (); }

  void set_directory (const std::string& dir)
    { get_rep ()->set_directory (dir); }

  std::string get_directory (void) const
    { return get_rep ()->get_directory (); }

  std::string get_name (void) const
    { return get_rep ()->get_name (); }

  bool is_builtin (void) const
    { return get_directory ().empty (); }

  void delete_object (cdef_object obj)
    { get_rep ()->delete_object (obj); }

  octave_value_list subsref_meta (const std::string& type,
                                  const std::list<octave_value_list>& idx,
                                  int nargout)
    { return get_rep ()->subsref_meta (type, idx, nargout); }

  static cdef_class make_meta_class (tree_classdef* t);

  octave_function* get_method_function (const std::string& nm);

  octave_function* get_constructor_function (void)
    { return get_method_function (get_name ()); }

  octave_value construct (const octave_value_list& args)
    { return get_rep ()->construct (args); }

  void initialize_object (cdef_object& obj)
    { get_rep ()->initialize_object (obj); }

  void run_constructor (cdef_object& obj, const octave_value_list& args)
    { get_rep ()->run_constructor (obj, args); }

  void mark_as_handle_class (void)
    { get_rep ()->mark_as_handle_class (); }

  bool is_handle_class (void) const
    { return get_rep ()->is_handle_class (); }

  static const cdef_class& meta_class (void) { return _meta_class; }
  static const cdef_class& meta_property (void) { return _meta_property; }
  static const cdef_class& meta_method (void) { return _meta_method; }
  static const cdef_class& meta_package (void) { return _meta_package; }

  void register_object (void) { get_rep ()->register_object (); }

  void unregister_object (void) { get_rep ()->unregister_object (); }

private:
  cdef_class_rep* get_rep (void)
    { return dynamic_cast<cdef_class_rep *> (cdef_object::get_rep ()); }
  
  const cdef_class_rep* get_rep (void) const
    { return dynamic_cast<const cdef_class_rep *> (cdef_object::get_rep ()); }

  friend bool operator == (const cdef_class&, const cdef_class&);
  friend bool operator != (const cdef_class&, const cdef_class&);
  friend bool operator < (const cdef_class&, const cdef_class&);

private:
  static cdef_class _meta_class;
  static cdef_class _meta_property;
  static cdef_class _meta_method;
  static cdef_class _meta_package;

  friend void install_classdef (void);
};

inline bool
operator == (const cdef_class& clsa, const cdef_class& clsb)
// FIXME: is this really the right way to check class equality?
{ return (clsa.get_rep () == clsb.get_rep ()); }

inline bool
operator != (const cdef_class& clsa, const cdef_class& clsb)
{ return ! (clsa == clsb); }

// This is only to be able to use cdef_class as map keys.
inline bool
operator < (const cdef_class& clsa, const cdef_class& clsb)
{ return clsa.get_rep () < clsb.get_rep (); }

class
cdef_property : public cdef_object
{
private:

  class
  cdef_property_rep : public handle_cdef_object
  {
  public:
    cdef_property_rep (void)
	: handle_cdef_object () { }

    bool is_property (void) const { return true; }

    std::string get_name (void) const { return get("Name").string_value (); }

    void set_name (const std::string& nm) { put ("Name", nm); }

    bool is_constant (void) const { return get("Constant").bool_value (); }

    octave_value get_value (void) const { return default_value; }

    octave_value get_value (const cdef_object& obj);

    void set_value (const octave_value& val) { default_value = val; }

    void set_value (cdef_object& obj, const octave_value& val);

  private:
    bool is_recursive_set (const cdef_object& obj) const;

  private:
    octave_value default_value;
  };

public:
  cdef_property (void) : cdef_object () { }

  cdef_property (const std::string& nm)
      : cdef_object (new cdef_property_rep ())
    { get_rep ()->set_name (nm); }

  cdef_property (const cdef_property& prop)
      : cdef_object (prop) { }

  cdef_property (const cdef_object& obj)
      : cdef_object (obj)
    {
      // This should never happen...
      if (! is_property ())
	error ("internal error: invalid assignment from %s to meta.property object",
	       class_name ().c_str ());
    }

  cdef_property& operator = (const cdef_property& prop)
    {
      cdef_object::operator= (prop);

      return *this;
    }

  octave_value get_value (const cdef_object& obj)
    { return get_rep ()->get_value (obj); }

  octave_value get_value (void) { return get_rep ()->get_value (); }

  void set_value (cdef_object& obj, const octave_value& val)
    { get_rep ()->set_value (obj, val); }

  void set_value (const octave_value& val) { get_rep ()->set_value (val); }
 
  bool check_get_access (void) const;
  
  bool check_set_access (void) const;

  std::string get_name (void) const { return get_rep ()->get_name (); }

  bool is_constant (void) const { return get_rep ()->is_constant (); }

private:
  cdef_property_rep* get_rep (void)
    { return dynamic_cast<cdef_property_rep *> (cdef_object::get_rep ()); }
  
  const cdef_property_rep* get_rep (void) const
    { return dynamic_cast<const cdef_property_rep *> (cdef_object::get_rep ()); }
};

class
cdef_method : public cdef_object
{
private:

  class
  cdef_method_rep : public handle_cdef_object
  {
  public:
    cdef_method_rep (void) : handle_cdef_object () { }

    bool is_method (void) const { return true; }

    std::string get_name (void) const { return get("Name").string_value (); }

    void set_name (const std::string& nm) { put ("Name", nm); }

    bool is_static (void) const { return get("Static").bool_value (); }

    octave_value get_function (void) const { return function; }

    void set_function (const octave_value& fcn) { function = fcn; }

    octave_value_list execute (const octave_value_list& args, int nargout);

    octave_value_list execute (const cdef_object& obj,
			       const octave_value_list& args, int nargout);

    bool is_constructor (void) const;

  private:
    void check_method (void);

  private:
    octave_value function;
  };

public:
  cdef_method (void) : cdef_object () { }

  cdef_method (const std::string& nm)
      : cdef_object (new cdef_method_rep ())
    { get_rep ()->set_name (nm); }

  cdef_method (const cdef_property& prop)
      : cdef_object (prop) { }

  cdef_method (const cdef_object& obj)
      : cdef_object (obj)
    {
      // This should never happen...
      if (! is_method ())
	error ("internal error: invalid assignment from %s to meta.method object",
	       class_name ().c_str ());
    }

  cdef_method& operator = (const cdef_method& meth)
    {
      cdef_object::operator= (meth);

      return *this;
    }

  /* normal invokation */
  octave_value_list execute (const octave_value_list& args, int nargout)
    { return get_rep ()->execute (args, nargout); }

  /* dot-invokation: object is pushed as 1st argument */
  octave_value_list execute (const cdef_object& obj,
			     const octave_value_list& args, int nargout)
    { return get_rep ()->execute (obj, args, nargout); }

  bool check_access (void) const;
  
  std::string get_name (void) const { return get_rep ()->get_name (); }

  bool is_static (void) const { return get_rep ()->is_static (); }

  void set_function (const octave_value& fcn)
    { get_rep ()->set_function (fcn); }

  octave_value get_function (void) const
    { return get_rep ()->get_function (); }

  bool is_constructor (void) const
    { return get_rep ()->is_constructor (); }

private:
  cdef_method_rep* get_rep (void)
    { return dynamic_cast<cdef_method_rep *> (cdef_object::get_rep ()); }
  
  const cdef_method_rep* get_rep (void) const
    { return dynamic_cast<const cdef_method_rep *> (cdef_object::get_rep ()); }
};

inline cdef_class
cdef_object_rep::get_class (void) const
{
  gripe_invalid_object ("get_class");
  return cdef_class ();
}

inline std::string
cdef_object_rep::class_name (void) const
{ return get_class ().get_name (); }

inline cdef_class
cdef_object::get_class (void) const
{ return rep->get_class (); }

inline cdef_class
cdef_object_base::get_class (void) const
{ return cdef_class (klass); }

inline void
cdef_object_base::set_class (const cdef_class& cls)
{
  if ((klass.ok () && cls.ok () && cls != get_class ())
      || (klass.ok () && ! cls.ok ())
      || (! klass.ok () && cls.ok ()))
    {
      unregister_object ();
      klass = cls;
      register_object ();
    }
}

inline void
cdef_object_base::register_object (void)
{
  if (klass.ok ())
    {
      cdef_class cls (get_class ());

      if (! error_state && cls.ok ())
        cls.register_object ();
    }
}

inline void
cdef_object_base::unregister_object (void)
{
  if (klass.ok ())
    {
      cdef_class cls (get_class ());

      if (! error_state && cls.ok ())
        cls.unregister_object ();
    }
}

inline cdef_method
cdef_class::find_method (const std::string& nm, bool local)
{ return get_rep ()->find_method (nm, local); }

inline cdef_property
cdef_class::find_property (const std::string& nm)
{ return get_rep ()->find_property (nm); }

class
cdef_package : public cdef_object
{
private:

  class
  cdef_package_rep : public handle_cdef_object
  {
  public:
    cdef_package_rep (void) : handle_cdef_object () { }

    bool is_package (void) const { return true; }

    std::string get_name (void) const { return get("Name").string_value (); }

    void set_name (const std::string& nm) { put ("Name", nm); }

    void install_class (const cdef_class& cls, const std::string& nm);

    void install_function (const octave_value& fcn, const std::string& nm);

    void install_package (const cdef_package& pack, const std::string& nm);

    Cell get_classes (void) const;

    Cell get_functions (void) const;

    Cell get_packages (void) const;

  private:
    std::map<std::string, cdef_class> class_map;
    std::map<std::string, octave_value> function_map;
    std::map<std::string, cdef_package> package_map;

    typedef std::map<std::string, cdef_class>::iterator class_iterator;
    typedef std::map<std::string, cdef_class>::const_iterator class_const_iterator;
    typedef std::map<std::string, octave_value>::iterator function_iterator;
    typedef std::map<std::string, octave_value>::const_iterator function_const_iterator;
    typedef std::map<std::string, cdef_package>::iterator package_iterator;
    typedef std::map<std::string, cdef_package>::const_iterator package_const_iterator;
  };

public:
  cdef_package (void) : cdef_object () { }

  cdef_package (const std::string& nm)
      : cdef_object (new cdef_package_rep ())
    { get_rep ()->set_name (nm); }

  cdef_package (const cdef_object& obj)
      : cdef_object (obj)
    {
      // This should never happen...
      if (! is_package ())
	error ("internal error: invalid assignment from %s to meta.package object",
	       class_name ().c_str ());
    }

  cdef_package& operator = (const cdef_package& pack)
    {
      cdef_object::operator= (pack);

      return *this;
    }

  void install_class (const cdef_class& cls, const std::string& nm)
    { get_rep ()->install_class (cls, nm); }

  void install_function (const octave_value& fcn, const std::string& nm)
    { get_rep ()->install_function (fcn, nm); }

  void install_package (const cdef_package& pack, const std::string& nm)
    { get_rep ()->install_package (pack, nm); }

  Cell get_classes (void) const
    { return get_rep ()->get_classes (); }

  Cell get_functions (void) const
    { return get_rep ()->get_functions (); }

  Cell get_packages (void) const
    { return get_rep ()->get_packages (); }

  std::string get_name (void) const { return get_rep ()->get_name (); }

  static const cdef_package& meta (void) { return _meta; }

private:
  cdef_package_rep* get_rep (void)
    { return dynamic_cast<cdef_package_rep *> (cdef_object::get_rep ()); }
  
  const cdef_package_rep* get_rep (void) const
    { return dynamic_cast<const cdef_package_rep *> (cdef_object::get_rep ()); }

private:
  static cdef_package _meta;

  friend void install_classdef (void);
};

class
octave_classdef : public octave_base_value
{
public:
  octave_classdef (void)
      : octave_base_value (), object () { }

  octave_classdef (const cdef_object& obj)
      : octave_base_value (), object (obj) { }

  octave_classdef (const octave_classdef& obj)
      : octave_base_value (obj), object (obj.object) { }

  octave_base_value* clone (void) const
    { return new octave_classdef (object.clone ()); }

  octave_base_value* empty_clone (void) const
    { return new octave_classdef (); }

  cdef_object get_object (void) const { return object; }

  cdef_object& get_object_ref (void) { return object; }

  bool is_defined (void) const { return true; }

  bool is_map (void) const { return true; }

  bool print_as_scalar (void) const { return true; }

  void print(std::ostream& os, bool pr_as_read_syntax = false) const
    {
      // FIXME: should call "display" method
      print_raw(os, pr_as_read_syntax);
      newline(os);
    }

  void print_raw(std::ostream& os, bool /* pr_as_read_syntax */ = false) const
    {
      os << object.class_name () << " object";
    }

  octave_value_list subsref (const std::string& type,
			     const std::list<octave_value_list>& idx, int nargout);

  octave_value subsref (const std::string& type,
			const std::list<octave_value_list>& idx)
    {
      octave_value_list retval = subsref (type, idx, 1);
      return (retval.length () > 0 ? retval(0) : octave_value ());
    }

  octave_value subsasgn (const std::string& type,
                         const std::list<octave_value_list>& idx,
                         const octave_value& rhs);

  string_vector map_keys (void) const { return object.map_keys (); }

  dim_vector dims (void) const { return dim_vector (1, 1); }

private:
  cdef_object object;

private:
  DECLARE_OCTAVE_ALLOCATOR

public:
  int type_id (void) const { return t_id; }
  std::string type_name (void) const { return t_name; }
  std::string class_name (void) const { return object.class_name (); }

  static int static_type_id (void) { return t_id; }
  static std::string static_type_name (void) { return t_name; }
  static std::string static_class_name (void) { return "<unknown>"; }
  static void register_type (void);

private:
  static int t_id;

  static const std::string t_name;
};

inline octave_value
to_ov (const cdef_object& obj)
{
  if (obj.ok ())
    return octave_value (new octave_classdef (obj));
  else
    return octave_value (Matrix ());
}

inline octave_value
to_ov (const octave_value& ov)
{ return ov; }

inline cdef_object
to_cdef (const octave_value& val)
{
  if (val.type_name () == "object")
    return dynamic_cast<octave_classdef *> (val.internal_rep ())->get_object ();
  else
    {
      warning ("trying to cast non-object into object");
      return cdef_object ();
    }
}

inline cdef_object&
to_cdef_ref (octave_value& val)
{
  static cdef_object empty;

  if (val.type_name () == "object")
    return dynamic_cast<octave_classdef *> (val.internal_rep ())->get_object_ref ();
  else
    {
      warning ("trying to cast non-object into object");
      return empty;
    }
}

inline cdef_object
to_cdef (const cdef_object& obj)
{ return obj; }

OCTINTERP_API void install_classdef (void);

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/

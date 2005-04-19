/*
    Copyright (C) 2005 by Eric Sunshine <sunshine@sunshineco.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSUTIL_VERBOSITY_H__
#define __CS_CSUTIL_VERBOSITY_H__

/**\file
 * Verbosity management helpers
 */
#include "csextern.h"
#include "iutil/verbositymanager.h"
#include "csutil/strhash.h"
#include "csutil/csstring.h"
class csStringArray;

/**
 * Utility for parsing verbosity flags such as those provided by the
 * <tt>--verbose=flags</tt> command-line option.  The general format of a
 * verbosity flag is an optional <tt>+</tt> or <tt>-</tt> followed by a
 * period-delimited name, such as <tt>foo.bar.baz</tt>.  Multiple flags can be
 * specified, separated by commas. The <tt>+</tt> and <tt>-</tt> tokens
 * indicate whether the verbosity class should be enabled or disabled. If
 * neither is specified, then <tt>+</tt> is assumed.  The <tt>flags</tt>
 * argument given to the csVerbosityManager constructor and the Parse() method
 * is the text following the equal sign in the command-line
 * <tt>--verbose=flags</tt> option; it should not include the literal
 * "<tt>--verbose=</tt>".
 * <p>
 * Period-delimited verbosity names form a sort of parent/child relationship
 * hierarchy which makes it possible to specify both coarse- and fine-grained
 * verbosity settings easily. When the Enabled() method tests if a verbosity
 * class is enabled, it first checks the most specific child flag. If no such
 * flag is found, it tries a less specific option by backing up to the child's
 * parent.  If that fails, it attempts the grandparent, and so on.  For
 * example, given the command-line verbosity setting `<tt>-foo,+foo.bar</tt>',
 * if client code invokes <tt>Enabled("foo.bar.baz")</tt>, it will first look
 * for <tt>foo.bar.baz</tt>. When that fails to produce a result, it will look
 * for <tt>foo.bar</tt>, which will succeed, reporting that verbosity is
 * enabled for <tt>foo.bar.baz</tt> on account of the explicit
 * <tt>+foo.bar</tt> from the command-line. Thus, though not specified
 * explicitly, <tt>foo.bar.baz</tt> implicitly inherits the setting of its
 * parent <tt>foo.bar</tt>.  Likewise, <tt>Enabled("foo.beep.bop")</tt> will
 * attempt, in order, <tt>foo.beep.bop</tt>, then <tt>foo.beep</tt>, and
 * ultimately <tt>foo</tt>, which finally succeeds, reporting that verbosity is
 * disabled for <tt>foo.beep.bop</tt> on account of the <tt>-foo</tt> from the
 * command-line.
 * <p>
 * If an invocation of Enabled() is unable to find any matching verbosity
 * class, even after traveling up the inheritance hierarchy, it instead reports
 * the global verbosity setting.  The global verbosity setting is derived
 * inversely from the very first verbosity specification seen by the
 * constructor or by Parse(). If the first verbosity flag is `true' (via the
 * optional <tt>+</tt>), then the global verbosity setting is
 * `false'. Conversely, if the first verbosity flag is `false' (via
 * <tt>-</tt>), then the global setting is `true'. The upshot of this heuristic
 * is that command-line verbosity settings work in an intuitive fashion. For
 * instance, <tt>--verbose=scf</tt> (or <tt>+scf</tt>) enables verbosity for
 * only the `scf' module and disables it for all other modules. Likewise,
 * <tt>--verbose=-scf</tt> enables verbosity for all modules except `scf'.
 * <p>
 * If only the empty-string "" is passed to the constructor or Parse(), then
 * Enabled() returns `true' for all queries. This gives the intuitive
 * interpretation to a bare <tt>--verbose</tt> without any flags, which means
 * to enable verbosity for all modules. If only a null is passed to the
 * constructor or Parse(), then Enabled() returns `false' for all queries. This
 * is interpreted intuitively as meaning that no modules should be verbose
 * in the absence of <tt>--verbose</tt> on the command-line.
 * <p>
 * Examples:
 * <ul>
 * <li><tt>--verbose</tt> Enables verbosity for all modules.</li>
 * <li><tt>--verbose=scf,vfs</tt> Enables verbosity for only `scf' and
 *     `vfs'.</li>
 * <li><tt>--verbose=-scf,-vfs</tt> Enables verbosity for all modules except
 *     `scf' and `vfs'.</li>
 * <li><tt>--verbose=+scf.register.class</tt> Enables verbosity for only
 *     `scf.register.class'.</li>
 * <li><tt>--verbose=+scf,-scf.register,+scf.register.class</tt> Enables
 *     verbosity for only `scf' and its children except `scf.register' or any
 *     of its children other than `scf.register.class which is explicitly
 *     enabled.</li>
 * </ul>
 * \sa csVerbosityManager
 * \sa csCheckVerbosity
 * \sa csParseVerbosity;
 * \sa iVerbosityManager
 */
class CS_CRYSTALSPACE_EXPORT csVerbosityParser
{
private:
  csStringHash flags;

  typedef csStringFast<128> Str;
  typedef bool (*SplitPredicate)(char c, size_t pos);
  static bool Split(char const*, char delim, SplitPredicate, bool empty_okay,
		    csStringArray&);
  static Str  Join(csStringArray const&, Str delim);
  static bool Error(char const* msg, char const* s, size_t pos);
  static bool ParseToggle(char const*&);
  static bool ParseFlag(char const*, csStringArray&, bool empty_okay);
  bool TestFlag(Str name, bool& enable) const;
  void Copy(csVerbosityParser const& v) { flags = v.flags; }

public:
  /**
   * Construct the verbosity flag parser.
   * \param flags The verbosity flags; usually the text following the equal
   *   sign in the command-line `<tt>--verbose=flags</tt> option.
   * \remarks If \a flags is the empty (zero-length) string "", then all
   *   Enabled() queries return true. This is the intuitive interpretation of a
   *   bare <tt>--verbose</tt> option without any explicit flags specification.
   *   If \a flags is a null pointer, then all Enabled() queries return false.
   *   This is the intuitive interpretation of the absence of
   *   <tt>--verbose</tt> on the command-line.
   * <p>
   * \remarks See the class description for detailed information regarding the
   *   interpretation of \a flags.
   */
  csVerbosityParser(char const* flags = 0);
  
  /**
   * Copy constructor.
   */
  csVerbosityParser(csVerbosityParser const& v) { Copy(v); }

  /**
   * Destructor.
   */
  ~csVerbosityParser() {}

  /**
   * Assignment operator.
   */
  csVerbosityParser& operator=(csVerbosityParser const& v)
  { Copy(v); return *this; }

  /**
   * Parse additional verbosity flags.
   * \param flags The verbosity flags; usually the text following the equal
   *   sign in the command-line `<tt>--verbose=flags</tt> option.
   * \remarks This method is useful if additional flags need to be parsed after
   *   construction time.
   * <p>
   * \remarks See the class description for detailed information regarding the
   *   interpretation of \a flags.
   */
  void Parse(char const* flags);

  /**
   * Check if verbosity should be enabled for a particular flag.
   * \param flag The flag for which verboseness should be queried.  If \a flag
   *   is a null pointer, then the global verbosity setting is queried.  Such
   *   coarse-grained checking is not typically desired, though it may be
   *   useful in the unlikely event of a module needing to know if verbosity is
   *   enabled for any other (unspecified) modules.
   * \param fuzzy When true, if there is no exact match for \a flag, then also
   *   travel up the inheritance chain, checking the parent, grandparent, etc.
   *   This is the normal, desired behavior, and is the default setting.  There
   *   may be special cases, however, when traversing the inheritance chain
   *   should be avoided, and an explicit match of \a flag is warranted. For
   *   example, a module may want to provide a way to emit low-level debugging
   *   messages when requested explicitly, but not normally emit them when a
   *   general <tt>--verbose</tt> is in effect. In such cases, setting \a fuzzy
   *   to false would be appropriate.
   * <p>
   * \remarks See the class description for detailed information regarding the
   *   interpretation of \a flag.
   */
  bool Enabled(char const* flag = 0, bool fuzzy = true) const;

  /**
   * Given major and minor components, check if the verbosity class
   * "major.minor" is enabled.
   * \deprecated Use instead the more generic Enabled() method, which accepts
   *   any granularity of class breakdown; not just major and minor components.
   */
  CS_DEPRECATED_METHOD
  bool CheckFlag(char const* major, char const* minor) const
  {
    Str flag; flag << major << '.' << minor;
    return Enabled(flag);
  }
};

/**
 * Construct a csVerbosityParser from <tt>--verbosity=flags</tt> options given
 * on the command-line.
 * \param argc Command-line argument count from main().
 * \param argv Command-line argument vector from main().
 * \remarks This function extracts the <tt>flags</tt> text following the equal
 *   sign in each <tt>--verbose=flags</tt> option (if present) and constructs a
 *   csVerbosityParser with the extracted text.
 * \remarks This function is useful for verbosity queries during very early
 *   initialization before any of the higher-level framework has been set
 *   up. Once partial initialization has occurred, however, it is often more
 *   convenient to obtain a handle to an iVerbosityManager instance from the
 *   global iObjectRegitry and use that instead since it does not require
 *   access to \a argc and \a argv[].
 * \sa csVerbosityParser
 * \sa csVerbosityManager
 * \sa csCheckVerbosity
 * \sa iVerbosityManager
 */
CS_CRYSTALSPACE_EXPORT csVerbosityParser csParseVerbosity (
  int argc, char const* const argv[]);
  
/**
 * Search command-line arguments for <tt>--verbosity=flags</tt> options, and
 * check if \a flag is enabled or disabled.
 * \remarks This is a simple convenience wrapper around csCheckVerbosity()
 *   which invokes <tt>Enabled(flags)</tt> on the constructed verbosity parser.
 *   See the csParseVerbosity() and csVerbosityParser::Enabled() for a
 *   description of the arguments to this function.
 * \sa csVerbosityParser
 * \sa csVerbosityManager
 * \sa csParseVerbosity;
 * \sa iVerbosityManager
 */
CS_CRYSTALSPACE_EXPORT bool csCheckVerbosity (
  int argc, char const* const argv[], char const* flag = 0, bool fuzzy = true);
  
/**
 * Given major and minor components, check if the verbosity class
 * "major.minor" is enabled via the command-line `--verbose' switch.
 * \deprecated Use instead the more generic csCheckVerbosity() function which
 *   takes a single verbosity class flag since it accepts any granularity of
 *   class breakdown; not just major and minor components.
 */
CS_DEPRECATED_METHOD
CS_CRYSTALSPACE_EXPORT bool csCheckVerbosity (
  int argc, char const* const argv[], char const* major, char const* minor);
  
/**
 * Default iVerbosityManager implementation. Basically a thin wrapper around
 * csVerbosityParser. An instance of iVerbosityManager can be extracted from
 * the global iObjectRegistry once csInitializer::CreateEnvironment() or
 * csInitializer::CreateVerbosityManager() has been invoked.
 * \sa csVerbosityParser
 * \sa csCheckVerbosity
 * \sa csParseVerbosity;
 * \sa iVerbosityManager
 */
class CS_CRYSTALSPACE_EXPORT csVerbosityManager : public iVerbosityManager
{
private:
  csVerbosityParser vp;
public:
  SCF_DECLARE_IBASE;

  /**
   * Constructor.
   * \remarks See the csVerbosityParser constructor for detailed information
   *   regarding the interpretation of \a flags.
   */
  csVerbosityManager(char const* flags = 0) : vp(flags)
  { SCF_CONSTRUCT_IBASE(0); }
  /// Destructor.
  virtual ~csVerbosityManager() { SCF_DESTRUCT_IBASE(); }

  /**
   * Parse additional verbosity flags.
   * \remarks See csVerbosityParser::Parse() for detailed information
   *   regarding the interpretation of \a flags.
   */
  virtual void Parse(char const* flags) { vp.Parse(flags); }

  /**
   * Check if verbosity should be enabled for a particular flag.
   * \param flag The flag for which verboseness should be queried.
   * \param fuzzy Whether the search should match \a flag exactly (\a fuzzy =
   *   false) or if it can traverse the inheritance chain when searching for a
   *   match (\a fuzzy = true).
   * <p>
   * \remarks See the csVerbosityParser class description and
   *   csVerbosityParser::Enabled() for detailed information regarding the
   *   interpretation of \a flag and \a fuzzy.
   */
  virtual bool Enabled(char const* flag = 0, bool fuzzy = true) const
  { return vp.Enabled(flag, fuzzy); }

  /**
   * Given major and minor components, check if the verbosity class
   * "major.minor" is enabled.
   * \deprecated Use instead the more generic Enabled() method, which accepts
   *   any granularity of class breakdown; not just major and minor components.
   */
  CS_DEPRECATED_METHOD
  virtual bool CheckFlag(char const* major, char const* minor) const
  {
    csStringFast<128> flag; flag << major << '.' << minor;
    return Enabled(flag);
  }
};

#endif // __CS_CSUTIL_VERBOSITY_H__

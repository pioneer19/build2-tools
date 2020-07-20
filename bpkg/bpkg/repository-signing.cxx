// -*- C++ -*-
//
// This file was generated by CLI, a command line interface
// compiler for C++.
//

// Begin prologue.
//
#include <bpkg/types-parsers.hxx>
//
// End prologue.

#include <bpkg/repository-signing.hxx>

#include <map>
#include <cstring>

namespace bpkg
{
  ::bpkg::cli::usage_para
  print_bpkg_repository_signing_usage (::std::ostream& os, ::bpkg::cli::usage_para p)
  {
    CLI_POTENTIALLY_UNUSED (os);

    if (p != ::bpkg::cli::usage_para::none)
      os << ::std::endl;

    os << "\033[1mSYNOPSIS\033[0m" << ::std::endl
       << ::std::endl
       << "\033[1mbpkg rep-create --key\033[0m ...\033[0m" << ::std::endl
       << ::std::endl
       << "\033[1mDESCRIPTION\033[0m" << ::std::endl
       << ::std::endl
       << "The purpose of signing a repository is to prevent tampering with packages" << ::std::endl
       << "either during transmission or on the repository \033[4mhost machine\033[0m. Ideally, you" << ::std::endl
       << "would generate and sign the repository manifests on a separate \033[4mbuild machine\033[0m" << ::std::endl
       << "that is behind a firewall. This way, if (or, really, when) your host machine is" << ::std::endl
       << "compromised, it will be difficult for an attacker to compromise the repository" << ::std::endl
       << "packages without being noticed. Since the repository key is kept on the build" << ::std::endl
       << "machine (or, better yet, on a \033[4mone-way\033[0m PIV/PKCS#11 device; see below) they will" << ::std::endl
       << "not be able to re-sign the modified repository." << ::std::endl
       << ::std::endl
       << "\033[1mbpkg\033[0m uses X.509 public key cryptography for repository signing. Currently, only" << ::std::endl
       << "the explicit \033[4mfirst use\033[0m certificate authentication is implemented.  That is, for" << ::std::endl
       << "an unknown (to this \033[1mbpkg\033[0m configuration) repository certificate its subject" << ::std::endl
       << "information and fingerprint are presented to the user. If the user confirms the" << ::std::endl
       << "authenticity of the certificate, then it is added to the configuration and any" << ::std::endl
       << "repository that in the future presents this certificate is trusted without" << ::std::endl
       << "further confirmations, provided its name matches the certificate's subject (see" << ::std::endl
       << "below). In the future a certificate authority (CA)-based model may be added." << ::std::endl
       << ::std::endl
       << "The rest of this guide shows how to create a key/certificate pair for \033[1mpkg\033[0m" << ::std::endl
       << "repository signing and use it to sign a repository. At the end it also briefly" << ::std::endl
       << "explains how to store the private key on a PIV/PKCS#11 device using Yubikey 4" << ::std::endl
       << "as an example." << ::std::endl
       << ::std::endl
       << "1. Generate Private Key" << ::std::endl
       << ::std::endl
       << "    The first step is to generate the private key:" << ::std::endl
       << ::std::endl
       << "    $ openssl genrsa -aes256 2048 >key.pem" << ::std::endl
       << ::std::endl
       << "    If you would like to generate a key without password protection (not a good" << ::std::endl
       << "    idea except for testing), leave the \033[1m-aes256\033[0m option out. You may also need" << ::std::endl
       << "    to add \033[1m-nodes\033[0m depending on your \033[1mopenssl(1)\033[0m configuration." << ::std::endl
       << ::std::endl
       << "2. Generate Certificate" << ::std::endl
       << ::std::endl
       << "    Next create the certificate configuration file by saving the following into" << ::std::endl
       << "    \033[1mcert.conf\033[0m. You may want to keep it around in case you need to renew an" << ::std::endl
       << "    expired certificate, etc." << ::std::endl
       << ::std::endl
       << "    name  = example.com" << ::std::endl
       << "    org   = Example, Inc" << ::std::endl
       << "    email = admin@example.com" << ::std::endl
       << ::std::endl
       << "    [req]" << ::std::endl
       << "    distinguished_name = req_distinguished_name" << ::std::endl
       << "    x509_extensions    = v3_req" << ::std::endl
       << "    prompt             = no" << ::std::endl
       << "    utf8               = yes" << ::std::endl
       << ::std::endl
       << "    [req_distinguished_name]" << ::std::endl
       << "    O  = $org" << ::std::endl
       << "    CN = name:$name" << ::std::endl
       << ::std::endl
       << "    [v3_req]" << ::std::endl
       << "    keyUsage         = critical,digitalSignature" << ::std::endl
       << "    extendedKeyUsage = critical,codeSigning" << ::std::endl
       << "    subjectAltName   = email:$email" << ::std::endl
       << ::std::endl
       << "    Adjust the first three lines to match your details. If the repository is" << ::std::endl
       << "    hosted by an organization, use the organization's name for \033[1morg\033[0m. If you host" << ::std::endl
       << "    it as an individual, put your full, real name there. Using any kind of" << ::std::endl
       << "    aliases or nicknames is a bad idea (except, again, for testing). Remember," << ::std::endl
       << "    users of your repository will be presented with this information and if" << ::std::endl
       << "    they see it was signed by someone named SmellySnook, they will unlikely" << ::std::endl
       << "    trust it. Also use a working email address in case users need to contact" << ::std::endl
       << "    you about issues with your certificate. Note that the \033[1mname:\033[0m prefix in the" << ::std::endl
       << "    \033[1mCN\033[0m value is not a typo." << ::std::endl
       << ::std::endl
       << "    The \033[1mname\033[0m field is a canonical repository name prefix with the \033[1mpkg:\033[0m type" << ::std::endl
       << "    part stripped. Any repository with a canonical name that starts with this" << ::std::endl
       << "    prefix can be authenticated by this certificate (see the repository" << ::std::endl
       << "    manifest documentation for more information on canonical names). For" << ::std::endl
       << "    example, name \033[1mexample.com\033[0m will match any repository hosted on" << ::std::endl
       << "    \033[1m{,www.,pkg.,bpkg.}example.com\033[0m. While name \033[1mexample.com/math\033[0m will match" << ::std::endl
       << "    \033[1m{...}example.com/pkg/1/math\033[0m but not \033[1m{...}example.com/pkg/1/misc\033[0m." << ::std::endl
       << ::std::endl
       << "    A certificate name can also contain a subdomain wildcard. A wildcard name" << ::std::endl
       << "    in the \033[1m*.example.com\033[0m form matches any single-level subdomain, for example" << ::std::endl
       << "    \033[1mfoo.example.com\033[0m but not \033[1mfoo.bar.example.com\033[0m while a wildcard name in the" << ::std::endl
       << "    \033[1m**.example.com\033[0m form matches any subdomain, including multi-level. The above" << ::std::endl
       << "    two forms do not match the domain itself (\033[1mexample.com\033[0m in the above" << ::std::endl
       << "    example). If this is desired, the \033[1m*example.com\033[0m and \033[1m**example.com\033[0m forms" << ::std::endl
       << "    should be used instead. Note that these forms still only match subdomains." << ::std::endl
       << "    In other words, they won't match \033[1mfooexample.com\033[0m. Wildcard names are less" << ::std::endl
       << "    secure and therefore are normally only used for testing and/or internal" << ::std::endl
       << "    repositories." << ::std::endl
       << ::std::endl
       << "    Once the configuration file is ready, generate the certificate:" << ::std::endl
       << ::std::endl
       << "    openssl req -x509 -new -sha256 -key key.pem \\" << ::std::endl
       << "      -config cert.conf -days 730 >cert.pem" << ::std::endl
       << ::std::endl
       << "    To verify the certificate information, run:" << ::std::endl
       << ::std::endl
       << "    openssl x509 -noout -nameopt RFC2253,sep_multiline \\" << ::std::endl
       << "      -subject -dates -email <cert.pem" << ::std::endl
       << ::std::endl
       << "3. Add Certificate to Repository" << ::std::endl
       << ::std::endl
       << "    Add the \033[1mcertificate:\033[0m field for the base repository (\033[1mrole: base\033[0m) in the" << ::std::endl
       << "    \033[1mrepositories\033[0m manifest file(s):" << ::std::endl
       << ::std::endl
       << "    certificate: \\" << ::std::endl
       << "    <cert>" << ::std::endl
       << "    \\" << ::std::endl
       << ::std::endl
       << "    Replace \033[4mcert\033[0m with the entire contents of \033[1mcert.pem\033[0m (including the BEGIN" << ::std::endl
       << "    CERTIFICATE\033[0m and END CERTIFICATE\033[0m lines). So you will have an entry like" << ::std::endl
       << "    this:" << ::std::endl
       << ::std::endl
       << "    certificate: \\" << ::std::endl
       << "    -----BEGIN CERTIFICATE-----" << ::std::endl
       << "    MIIDQjCCAiqgAwIBAgIJAIUgsIqSnesGMA0GCSqGSIb3DQEBCwUAMDkxFzAVBgNV" << ::std::endl
       << "    ." << ::std::endl
       << "    ." << ::std::endl
       << "    ." << ::std::endl
       << "    +NOVBamEvjn58ZcLfWh2oKee7ulIZg==" << ::std::endl
       << "    -----END CERTIFICATE-----" << ::std::endl
       << "    \\" << ::std::endl
       << ::std::endl
       << "4. Sign Repository" << ::std::endl
       << ::std::endl
       << "    When generating the repository manifests with the \033[1mbpkg-rep-create(1)\033[0m" << ::std::endl
       << "    command, specify the path to \033[1mkey.pem\033[0m with the \033[1m--key\033[0m option:" << ::std::endl
       << ::std::endl
       << "    bpkg rep-create --key /path/to/key.pem /path/to/repository" << ::std::endl
       << ::std::endl
       << "    You will be prompted for a password to unlock the private key." << ::std::endl
       << ::std::endl
       << "5. Using PIV/PKCS#11 Device" << ::std::endl
       << ::std::endl
       << "    This optional step shows how to load the private key into Yubikey 4 and" << ::std::endl
       << "    then use it instead of the private key itself for signing the repository." << ::std::endl
       << "    Note that you will need OpenSSL 1.0.2 or later for the signing part to" << ::std::endl
       << "    work." << ::std::endl
       << ::std::endl
       << "    First change the Yubikey MKEY, PUK, and PIN if necessary. You should" << ::std::endl
       << "    definitely do this if it still has the factory defaults. Then import the" << ::std::endl
       << "    private key and the certificate into Yubikey (replace \033[4mmkey\033[0m with the" << ::std::endl
       << "    management key):" << ::std::endl
       << ::std::endl
       << "    yubico-piv-tool --key=<mkey> -a import-key -s 9c <key.pem" << ::std::endl
       << "    yubico-piv-tool --key=<mkey> -a import-certificate -s 9c <cert.pem" << ::std::endl
       << ::std::endl
       << "    After this you will normally save the certificate/private key onto backup" << ::std::endl
       << "    media, store it in a secure, offline location, and remove the key from the" << ::std::endl
       << "    build machine." << ::std::endl
       << ::std::endl
       << "    To sign the repository with Yubikey specify the following options instead" << ::std::endl
       << "    of just \033[1m--key\033[0m as at step 4 (\"SIGN key\"\033[0m is the label for the slot 9c\033[0m private" << ::std::endl
       << "    key):" << ::std::endl
       << ::std::endl
       << "    bpkg rep-create                                                   \\" << ::std::endl
       << "      --openssl-option rsautl:-engine --openssl-option rsautl:pkcs11  \\" << ::std::endl
       << "      --openssl-option rsautl:-keyform --openssl-option rsautl:engine \\" << ::std::endl
       << "      --key \"pkcs11:object=SIGN%20key\" /path/to/repository" << ::std::endl;

    p = ::bpkg::cli::usage_para::text;

    return p;
  }
}

// Begin epilogue.
//
//
// End epilogue.

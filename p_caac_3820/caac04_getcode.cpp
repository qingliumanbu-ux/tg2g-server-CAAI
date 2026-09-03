/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    3.0
Date:		2012-2-9
Description:查询需新增的物料编码
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac04_getcode)
//-EP_SYSTEM_HEAD_END

int f_caac04_getcode(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{

	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount = 0;

	CString  sqlstr("");
	CString v_code = "";


	CDbCommand cmd_inq(conn);

	try
	{
		sqlstr = " select num_code mat_code,mat_cname||decode(scale,'无','',' ','',scale)||decode(tech_critr,'无','',' ','',tech_critr)||decode(sg_sign,'无','',' ','',sg_sign) mat_name ,measure_unit mat_unit,'1' valid_flag "
			" from tymirnqy "
			" where 1=1  "
			" and num_code not in (select mat_code from tcaac11) "
			" union all "
			" select distinct mat_code,mat_name ,'吨' mat_unit,'1' valid_flag "
			" from tsi0211 "
			" where 1=1  "
			" and substr(mat_type_code,0,2) in('30','20')  "
			" and mat_code not in (select mat_code from tcaac11) ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应

		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}